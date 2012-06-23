<?php

/**
 * Filter handler permettant limiter la vue à l'utilisateur courrant
 * 
 * Si l'utilisateur n'a pas certaines permissions sélectionnée, la vue
 * utilisant ce filtre va limiter les résultats retourner à l'utilisateur
 * courrant. Dans le cas contraire il désactivera le filtre.
 */
class filter_jesa_handler_user extends views_handler_filter_in_operator {
  /**
   * Constructeur
   */
  function construct() {
    parent::construct();
    $this->value_value = t('Is the logged in user (bypassed by some permissions)');
  }
  
  /**
   * Enumeration des permissions pouvant désactiver le filtre
   * 
   * Donne la liste des permissions autorisées à ne pas se limiter à l'utilisateur
   * '
   */
  function get_value_options() {
    if (!isset($this->value_options)) {
      $this->value_title = t('Permissions to bypass filter.');
      $options = array();
      foreach (filter_jesa_permission() as $key => $value) {
        $options[$key] = $value['title'];
      }
    //  var_dump($options);
      $this->value_options = $options;
    }
  }
  
  
  /**
   * Opérateur agissant sur la liste des permissions.
   * 
   * On ne permet qu'un type d'opérateur (est inclu dans) car
   * les autres ne sont pas nécessaires.
   */
  function operators () {
    //On limite les choix de l'opérateur à l'inclusion
    // des permissions
    $operators = array(
       'in' => array(
         'title' => t('Is one of'),
         'short' => t('in'),
         'short_single' => t('='),
         'method' => 'op_simple',
         'values' => 1,
       ),
    );
    return $operators; 
  }


  /**
   * Ajoute le filtre à la requête existante.
   * 
   * Si l'utilisateur n'a pas la permission requise, on ajoute
   * une condition limitant les données à l'utilisateur connecté.
   */
  function query() {
    $this->ensure_my_table();
    // controle afin de voir si l'utilisateur possède une des permissions
    //  nécessaires pour ne pas limiter la vue à l'utilisateur actif.
    $bypass_filter = FALSE;
    if (is_array($this->value)) {
      foreach ($this->value as $permission) {      
        $bypass_filter = $bypass_filter || user_access($permission);
      }
    }
    
    if ($bypass_filter == FALSE) {
      // L'utilisateur n'a pas le droit de voir plus que seulement 
      // ce qui est prévu pour lui-même => ajout d'une condition
      // pour sélectionner uniquement les records de cet utilisateur.
      $field = $this->table_alias . '.' . $this->real_field . ' ';
      $or = db_or();
      $or->condition($field, '***CURRENT_USER***', '=');  
  		$this->query->add_where($this->options['group'], $or);      
    }
  }
}
