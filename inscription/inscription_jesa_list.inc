<?php
/**
 *  Gestion des inscriptions par les parents de leurs enfants.
 *
 * Les page affichées sont les suivantes:
 *   - La liste des enfants liés à l'utilisateur connecté (créé par 
 * inscription_jesa_list()).
 *   - Le formulaire d'incription à un stage (inscription_jesa_register())
 * 
 * <H2>Autorisations</H2>
 * l'accès à la liste des enfants se fait par un lien dans le menu 
 * (inscription_jesa_menu()). Ce lien n'est activé que pour les rôles
 * ayant reçu l'access "create parents-children link" (à savoir les rôles 
 * parent et administrateur).@n
 * l'accès au formulaire d'inscription se fait uniquement pour les rôles
 * ayant reçu l'accès "create parents-children link" (à savoir les rôles 
 * parent et administrateur) et si l'utilisateur connecté a un lien avec 
 * le stagiaire (à savoir pour lequel un node parent - enfant existe).
 * 
 * <H2>Pages</H2>
 * <H3>Liste des enfants</H3>
 * L'accès principal se fait par la liste des enfants.
 * Elle permet aux utilisateurs connectés d'avoir la liste de ses enfants 
 * (à savoir les utilisateurs ayant un rôle de stagaire pour lequel un node 
 * parent - enfant existe).@n
 * Pour chaque enfant, le statut de l'inscription au prochain stage est
 * affiché ainsi qu'un lien permettant à l'utilisateur connecté d'inscrire 
 * cet enfant aux deux prochains stages. (Si l'enfant est inscrit aux deux 
 * prochains stages,  le lien ne s'affiche plus)
 *  @n
 * <H3>Formulaire d'inscription à un stage</H3>
 * Ce fomulaire est appelé à partir de la liste des enfants. Il permet au parent
 * d'inscrire son enfant au(x) prochain(s) stage(s) (pour l'instant uniquement
 * aux deux prochains stage).@n
 * Le formulaire (inscription_jesa_register()) reçoit comme argument le user ID
 * du stagiaire. Il va proposer le (ou les) stage(s) auxquel(s) le stagiaire
 * peut être inscrit.
 *
 *  @file      inscription_jesa_list.inc
 *  @author    Jean-Paul Scandariato
 *  @date      25/10/2011
 */

/**
 * Affichage de tous les stagiaires liés à l'utilisateur.
 *
 * Fonction de rappel (callback) du menu "mes inscriptions", c'est la page
 *  principale pour un parent afin de gérer l'inscription de ces enfants.
 */
function inscription_jesa_list() {
  $output =  'Cette page permet de gérer les inscriptions de vos enfants.<br/>'
    . 'Si votre enfant n&rsquo;apparait pas dans la liste ci-dessous,'
    . 'prenez contact avec le ' 
    . l('gestionnaire du stage', '/contact/Stage pour jeunes');
  $header = array(
    _inscription_jesa_include_class(t('Name'), 'jesa_inscription'),
    _inscription_jesa_include_class(t('Next event registration'), 'jesa_inscription'),
    _inscription_jesa_include_class(t('Registration status'), 'jesa_inscription'),
    _inscription_jesa_include_class(t('Actions'), 'jesa_inscription'),
    );
  $rows = array();
  try {
    global $user;
    $parent = new Parentelle(array('uid' => $user->uid));
    foreach ($parent->enfants as $enfant) {
      $row = array();
      $participant = $enfant['Stagiaire'];
      
      $row[] = l($participant->fullname, 'inscriptions/jeunes/'. $participant->uid . '/edit');
        $prochain_stage = _inscription_jesa_prochain_stage($participant->uid);
        if (empty($prochain_stage)) {
          $row[] = _inscription_jesa_include_class(t('none'), 'jesa_inscription');
          $statut = '';
        }
        else {
          $stage = node_load($prochain_stage->field_stagiaire_stage[LANGUAGE_NONE][0]['nid']);
          $row[] = _inscription_jesa_include_class($stage->title, 'jesa_inscription');
          $paiement = isset($prochain_stage->field_stagiaire_paiement[LANGUAGE_NONE][0]['value']) ? $prochain_stage->field_stagiaire_paiement[LANGUAGE_NONE][0]['value'] : FALSE;
          $formulaire = isset($prochain_stage->field_stagiaire_formulaire[LANGUAGE_NONE][0]['value']) ? $prochain_stage->field_stagiaire_formulaire[LANGUAGE_NONE][0]['value'] : FALSE;
          if ($paiement && $formulaire) {
            $statut = 'Inscrit';
          }
          else {
            $statut = 'En attente';
            if(!$formulaire) {
              $statut = $statut . ' du formulaire';
              if(!$paiement) {
                $statut = $statut . ' et';
              }
            }
            if(!$paiement) {
              $statut = $statut . ' du paiement';
            }
          }
          $statut = $statut . '.';
        }
        $row[]   = _inscription_jesa_include_class($statut, 'jesa_inscription');
        $links   = array();
        $stagiaire['uid'] = $participant->uid;
        $stages = _inscription_jesa_get_next_free_stages($stagiaire);
        if (!empty($stages)) {
          // On ne permet d'aller vers l'inscription que si le stagiaire n'est pas
          //  inscrits aux stages suivants
          $links[] = l(t('Register'), 'inscriptions/jeunes/gestion/'. $participant->uid . 
            '/add', array('query' => drupal_get_destination(),));
        }
        $row[]   = _inscription_jesa_include_class(implode('&nbsp;|&nbsp;', $links), 'jesa_inscription');
        $rows[]  = $row;
      
    }
  }
  catch (ErrorException $e) {
    drupal_set_message(
      t('Le syst&egrave;me a rencontr&eacute; un probl&egrave;me lors de la
          confection de la liste des participants (!msg).</br>
          Veuillez prendre contact avec le @responsable pour l&rsquo;inscription.',
          array('!msg' => $e->getMessage(), '@responsable' => l(t('event manager'), '/contact/Stage pour jeunes'))
      ),
      'error'
    );
  }

  $output .= theme_table(array(
    'header' => $header,
    'rows' => $rows,
    'attributes' =>  array(),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => '',
    'empty' => t('The list is empty.') . ' ' . l(t('Add a member.'), 'inscription_jeune/add'),
    )
  );

  return $output;
}

/**
 * Renvoie un array utilisé par theme_table permettant d'inclure la 
 * classe dans les attributs
 *
 **/
 function _inscription_jesa_include_class($data, $class) {
  return array('data' => $data, 'class' => array($class),);
 }

/**
 * Renvoie le prochain stage auquel le participant est inscrit
 * 
 * Le prochain stage doit être un stage d'au moins 7 jours dans le futur.
 *
 * @param $uid : identification de l'utilisateur pour lequel on recherche 
 * l'inscription
 * 
 * @return l'objet node correspondant au prochain stage auquel l'utilisateur 
 * est inscrit.
 * FALSE autrement.
 */
function _inscription_jesa_prochain_stage($uid) {
   $result = FALSE;
  $query = "
    SELECT 
      n.nid
    FROM 
            node n
      INNER JOIN field_data_field_stagiaire_user stu 
        ON    stu.entity_id   = n.nid
          AND stu.entity_type = :entity_type 
          AND stu.deleted     = :deleted
      INNER JOIN field_data_field_stagiaire_stage sts 
        ON     n.nid          = sts.entity_id 
          AND sts.entity_type = :entity_type 
          AND sts.deleted     = :deleted
      INNER JOIN field_data_field_stage_dates d
              ON    d.entity_type = :entity_type
                AND d.deleted     = :deleted
                AND d.entity_id   = sts.field_stagiaire_stage_nid   
    WHERE 
      n.status = :published
      AND n.type = :node_type
      AND stu.field_stagiaire_user_uid = :uid
      AND d.field_stage_dates_value > DATE_FORMAT(DATE_ADD(NOW(), INTERVAL 7 DAY),:format_date)
    ORDER BY
      d.field_stage_dates_value ASC
  ";      
  try {
    $results = db_query_range(
      $query,
      0,
      1,
      array(
        ':entity_type' => 'node',
        ':node_type'   => 'stagiaire',
        ':deleted'     => 0,              // qui n'est pas supprimé
        ':language'    => LANGUAGE_NONE,  //language indépendant
        ':published'   => NODE_PUBLISHED,              //publié
        ':uid'         => $uid,     
        ':format_date' => '%Y-%m-%dT00:00:00',        
      ),
      array('throw_exception' => TRUE,)
    );
  }
  catch(PDOException $e) {
    watchdog_exception('jesa inscription', $e, 'DB error of %type: !message '
      . 'in %function (line %line of %file).', array(), WATCHDOG_ALERT, NULL);
    throw new ErrorException(
      t('Error during database access.'),
      0,
      E_USER_NOTICE
    );
  }
  if ($results->rowCount() == 1) {
    $result = node_load($results->fetchField(0));
  }

  return $result;
}

/**
*  Constructeur du formulaire de l'inscription d'un stagiaire par le parent.
*
*  Le formulaire est prévu pour inscrire le stagiaire à un stage par 
*   un de ses parents
*  
*  @see <a href="http://api.drupal.org/api/drupal/developer--topics--forms_api_reference.html/7">Forms api reference</a>
*  @see <a href="http://drupal.org/node/37775">Form api</a>
*/
function inscription_jesa_register($form, &$form_state) {
  //Contôle de sécurité l'utilisateur connecté a-t'il un lien avec le stagiaire
  //La sécurité intégrée de Drupal n'autorise que les utilisateurs de type 
  //parent à se connecter (voir inscription_jesa_menu() dans le fichier module)
  //mais en manipulant le lien un parent pourrait avoir accès et inscrire un
  //stagiaire sans lien défini.
  if (!isset($form_state['build_info']['args'][0])) {
    drupal_access_denied();    
  }
  else {
    global $user;
    $stagiaire['uid'] = $form_state['build_info']['args'][0];
    $parent['uid'] = $user->uid;
    if (_inscription_jesa_check_lien($stagiaire, $parent, NULL) !== TRUE) {
      drupal_access_denied();
    }    
  }

  $stages = _inscription_jesa_get_next_free_stages($stagiaire);

  if (empty($stages)) {
    //aucun stage disponible dans le futur
    drupal_set_message(
      t('Sorry, there is no event available.'), 'warning');
  }
  else {
    // il y a au moins un stage dans le futur
    $form['#tree'] = TRUE;
    $form['user'] = array (
      '#type' => 'value',
      '#value' => $stagiaire['uid'],
    );      
    $form['description'] = array(
      '#type'   => 'item',
      '#title' => t('Select the event(s).'),
    );
    $form['stages'] = array(
      '#type'    => 'checkboxes',
      '#options' => $stages,
      '#title'   => t('Events'),
    );
    $selection = array(
      'train' => array('required' => FALSE,),
    );
    $form += _inscription_jesa_get_form_incscription_fields($selection);
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );
    $form['cancel'] = array(
      '#type' => 'submit',
      '#value' => t('Cancel'),
    );
  }

  return $form;
}
 
/**
 * Validation de l'inscription.
 *
 * On contrôle si au moins un stage est sélectionné.
 *
 * Fonction de validation de inscription_jesa_register()
 */
function inscription_jesa_register_validate($form, &$form_state) {

  foreach ($form_state['values']['stages'] as $stage) {
    if ($stage) {
      //au moins un stage sélectionné => tout est correct
      return;
    }
  }
  //Nous avons itéré sur tous les stages et aucun n'a été sélectionné => erreur
  form_set_error('stages' ,t('Please select at least one event.'));  
} 
 
/**
 * Traitement du formulaire d'inscription.
 *
 * Crée un node de type stagiaire.
 *
 * Fonction de traitement de inscription_jesa_register()
 */
function inscription_jesa_register_submit($form, &$form_state) {
   foreach ($form_state['values']['stages'] as $stage) {
    if ($stage) {
      _inscription_jesa_set_incription(
        $form_state['values']['user'], 
        $stage, 
        $form_state['values']['train'], 
        $form_state['values']['annee'],
        0
      );
    }
  }
}

/**
 * Donne la liste des  prochains stages pour jeunes auxquels le stagiaire peut 
 * s'inscrire.
 *
 * On ne regarde que les deux prochains stages et les stages sélectionnés 
 * doivent au moins être une semaine dans le futur.
 *
 * @param array $stagiaire un tableau associatif contenant 
 *    - uid    : l'identifiant du stagiaire.
 *
 * @return un tableau associatif (nodeid => date du stage) des stages trouvés.
 *   Un tableau vide si aucun stage n'est trouvé.
 *
 */
function _inscription_jesa_get_next_free_stages($stagiaire) {
  $stages = _inscription_jesa_get_next_stages(2, 7);
  foreach ($stages as $stage_nid => $title) {
    if (_inscription_jesa_is_inscrit($stagiaire, $stage_nid)) {
      //Le stagiaire est déjà inscrit au stage => on retire le stage de la liste
      unset($stages[$stage_nid]);
    }
  }
  return $stages;
}