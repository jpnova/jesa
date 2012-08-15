<?php
/**
 * Module de gestion des usilisateurs
 *
 *  @file      jesa_obj
 *  @author    Jean-Paul Scandariato
 *  @date      25/10/2011
 */
 
 /**
 * Classe ancetre permettant l'accès dynamique des propriétés
 */
  abstract class PropertyObject {
    public function __get($name)
    {
      if (method_exists($this, ($method = 'get_'.$name)))
      {
        return $this->$method();
      }
      else return;
    }
    
    public function __isset($name)
    {
      if (method_exists($this, ($method = 'isset_'.$name)))
      {
        return $this->$method();
      }
      else return;
    }
    
    public function __set($name, $value)
    {
      if (method_exists($this, ($method = 'set_'.$name)))
      {
        $this->$method($value);
      }
    }
    
    public function __unset($name)
    {
      if (method_exists($this, ($method = 'unset_'.$name)))
      {
        $this->$method();
      }
    }
  }
 
   class Utilisateur extends PropertyObject {
    
  
    /**
    *  Objet drupal représentant l'utilisateur
    * @private
    */
    private $account;
    /**
    * Identidifiant de l'utilistateur
    * @public
    */
    public $uid = 0;
    /**
    * Constructeur, crée l'obje Utilisateur
    * 
    * 
    * @param array $args Tableau associatif contenant les arguments pour la création de l'objet. Les valeurs possibles sont:
    *   -@c 'uid'         Identifiant de l'utilisateur
    *   -@c 'user_name'    Nom d'utilisateur
    *      
    * 
    */  
    function __construct($args = null) {
      if(is_array($args) && !empty($args)) {
  
  			//Action(s) distincte(s) pour un paramètre.
  			if(!empty($args['uid'])) {
          $this->uid = $args['uid'];
  			}
      }
    }
    
    private function load_account() {
      if ($this->uid >> 0) {
        $this->account = user_load($this->uid);
      }
    }
    
    protected function get_account() {
      if (empty($this->account)) {
        $this->load_account();
      }
      return $this->account;
    }

    protected function get_fullname() {
      return  $this->get_firstname() . ' ' . $this->get_lastname();
      //return $fullname;
    }
    
    protected function get_firstname() {
      $prenom = field_get_items('user',$this->get_account(),'field_prenom');
      return $prenom[0]['safe_value'];  
    }
    
    protected function get_lastname() {
      $nom = field_get_items('user',$this->get_account(),'field_nom');
      return $nom[0]['safe_value'];  
    }
  }
  
  class Parentelle extends Utilisateur {
    private $enfants = array();
    
    protected function get_Enfants() { 
      if (empty($this->enfants)) {
        $query = new EntityFieldQuery();
    
        $query->entityCondition('entity_type', 'node')
          ->entityCondition('bundle', 'parent_stagiaire')
          // ->propertyCondition('status', 1)
          // ->range(0, 10)
          ->addMetaData('account', user_load(1));
        try {
          $results = $query->execute();
        }    
        catch(PDOException $e) {
          watchdog_exception(
            'jesa inscription', 
            $e, 
            'DB error of %type: !message in %function (line %line of %file).', 
            array(), 
            WATCHDOG_ALERT, NULL
          );
          throw new ErrorException(
            t('Error during database access.'),
            0,
            E_USER_NOTICE
          );
        }     
        if (!empty($results['node'])) {
          foreach ($results['node'] as $record){
            $node_parent_enfant = node_load($record->nid);
            $enfant = field_get_items('node',$node_parent_enfant,'field_lien_enfant');
            $stagiaire = new Stagiaire($enfant[0]);
            if ($stagiaire->account === FALSE) {
              //la reference pointe sur un Utilisateur qui n'esxiste plus => supprimer le lien
              node_delete($record->nid);
            } 
            else {
              $lien = field_get_items('node',$node_parent_enfant,'field_lien_type');
              $this->enfants[]=array('Stagiaire'=>$stagiaire, 'Lien'=>$lien[0], 'nid'=>$record->nid);
            }
          }            
        }          
      }                   
      return $this->enfants;
    }
  }
  
  
  /**
  *  Stagiaire : Utilisateur ayant le role stagiaire
  * 
  *  Cet utilisateur participes aux stages 
  */ 
  class Stagiaire extends Utilisateur {
    private $inscriptions = array();
  }
  
  
  /**
  *
  * Information sur le stage
  * 
  */
  class Stages {
    /**
    * Liste des stages déjà recherchés par l'une ou l'autre requête.
    * 
    * Tableau associatif (nid => arrayinfo) où
    *   -@c nid : identifiant du node du stage
    *   -@c arrayinfo : tableau contenant les informations du stage:
    *   
    *  arrayinfo :
    *   -@c titre  : Titre du stage
    *   -@c public : public cible
    *   -@c debut  : date début (format Y-m-d\T00:00:00)
    *   -@c fin    : date début (format Y-m-d\T00:00:00)
    */
    protected $stages = array();
    /**
    * Contient l'identifiant (nid) du premier stage par public cible dans le futur
    * 
    * C'est un tableau associatif (key => value) ou key est le public cible ('jeunes','adultes')
    * et value contient le nid du premier stage dans le futur. 
    *
    */
    protected $Stage_futur_uid = array();
    /**
    * Donne la liste des prochains stages.
    *
    * Les stages sélectionnés doivent au moins être une semaine dans le futur.
    *
    * @param string $public_cible une des valeurs suivante:
    *   -@c Jeunes  : Ne renvoie que les stages pour jeunes
    *   -@c Adultes : Ne renvoie que les stages pour Adultes
    *   -@c all     : Renvoie tous les stages
    * @param integer $num_stages (optionel)nombre de stages à retourner dans la liste. Par
    * défaut a la valeur 1.
    * 
    *  
    * @return un tableau associatif (nodeid => array(titre, cible, debut, fin)) des stages trouvés
    *   triés par date début.
    *   Un tableau vide si aucun stage n'est trouvé.
    */
    function get_next_stages($public_cible, $num_stages=1) {
       //recherche dans les données déjà recherchées
       if (!empty($this->Stage_futur_uid[$public_cible])) {
         $index = 0;
         $stages_futurs = array();
         reset($this->stages);
         while(key($this->stages) != $this->Stage_futur_uid[$public_cible]) {
           next($this->stages);
         }
         do {
           $stage = array(key($this->stages) => current($this->stages));
           if ($stage[key($this->stages)]['public'] == $public_cible) {
             $stages_futurs += $stage;
             $index += 1;
           }
         }
         while (($index < $num_stages) && (next($this->stages) !== FALSE));
         //Controle si le nombre de stages trouvés en cache correspond
         // au nombre demandés
         if ($index < $num_stages) {
           //on va rechercher les stages manquants.
           $stage = end($stages_futurs);
           $stages_futurs += $this->query_next_stages($public_cible, $num_stages - $index, $stage['debut']);
           return $stages_futurs;
         }         
       }
       else {
         return $this->query_next_stages($public_cible, $num_stages);
       }
    }
    /**
    * Donne la liste des prochains stages.
    *
    * Va rechercher les informations dans la base de données (elle complémente la
    * fonction get_next_stages.)
    *
    * @param string $public_cible une des valeurs suivante:
    *   -@c Jeunes  : Ne renvoie que les stages pour jeunes
    *   -@c Adultes : Ne renvoie que les stages pour Adultes
    *   -@c all     : Renvoie tous les stages
    * @param integer $num_stages (optionel)nombre de stages à retourner dans la liste. Par
    * défaut a la valeur 1.
    * @param date $date (optionel) date à partir de laquelle on recherche les stages. Si la date
    *   est vide, on utilise la date du jour plus une semaine.
    * 
    * @return un tableau associatif (nodeid => debut) des stages trouvés
    *   triés par date début.
    *   Un tableau vide si aucun stage n'est trouvé.
    * 
    * @see get_next_stages
    */    
    private function query_next_stages($public_cible, $num_stages=1, $date_debut=null)  {
      $stages_futurs = array();
      $type_public = array();  //10 = 'jeunes', 9='adultes'
      switch ($public_cible) {
        case 'jeunes' :
          $type_public += array(10);
          break;
        case 'adultes' :
          $type_public += array(9);
          break;
        default :
          $type_public += array(9, 10);
      }
      
      $date_cible = empty($date_debut) ? date('Y-m-d\T00:00:00',strtotime(date('Y-m-d') . '+ 7 days')) : $date_debut;
      
      $query = new EntityFieldQuery();
      
      $query->entityCondition('entity_type', 'node')
        ->entityCondition('bundle', 'stage')
        ->propertyCondition('status', NODE_PUBLISHED)
        ->fieldCondition('field_stage_dates', 'value', $date_cible,  '>')
        ->fieldCondition('field_stage_public', 'tid', $type_public, 'IN')
        ->fieldOrderBy('field_stage_dates', 'value', 'ASC')
        ->range(0, $num_stages);
        
      try {
       $results = $query->execute();
      }
      catch(PDOException $e) {
        watchdog_exception('jesa inscription', $e, 'DB error of %type: !message in 
          %function (line %line of %file).', array(), WATCHDOG_ALERT, NULL);
        throw new ErrorException(
          t('Error during database access.'),
          0,
          E_USER_NOTICE
        );
      }
        
      if (!empty($results['node'])) {
        foreach ($results['node'] as $record){  
          $node = node_load($record->nid);
          $stages_futurs += array(
              $record->nid => array(              
                'titre'  => $node->title,
                'public' => field_extract_value('node', $node,'field_stage_public', 0)->name,
                'debut'  => field_extract_value('node', $node, 'field_stage_dates', 0, array('key' => 'value')),
                'fin'    => field_extract_value('node', $node, 'field_stage_dates', 0, array('key' => 'value2')),
            ),
          );
        }
        uasort($stages_futurs, 'custom_sort_stage_date');
        $cilbe = current($stages_futurs);
        if (empty($date_debut)) {
          $this->Stage_futur_uid = array($cilbe['public'] => key($stages_futurs));
        }
        if ($public_cible == 'all') {
          $first_type = $cilbe['public'];
          while (next($stages_futurs)!== FALSE) {
            $cilbe = current($stages_futurs);
            if ($cilbe['public'] != $first_type) {
              $this->Stage_futur_uid += array($cilbe['public'] => key($stages_futurs));
              break;
            }
          }
        }
        $this->stages += $stages_futurs;
        uasort($this->stages, 'custom_sort_stage_date');
      }
      return $stages_futurs;
    }
  }
  
  
  /**
  *
  * Information sur le stage
  * 
  */
  class Stage {
    /**
    *  Public cible
    */
    protected $pulic;
    /**
    * node contenant les informations sur le stage
    */
    protected $node;
  }
  
  /**
  * Renvoie l'ordre de comparaison entre deux tableaux de type stage.
  * 
  * Utilisé dans query_next_stages
  * @see query_next_stages 
  */
  function custom_sort_stage_date($a,$b) {
    if ($a['debut'] == $b['debut']) {
        return 0;
    }
    return ($a['debut'] < $b['debut']) ? -1 : 1;
  }