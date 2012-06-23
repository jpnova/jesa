<?php
/**
 * @file
 * Provide views data and handlers for module ????
 */

/**
 * @defgroup filter_jesa_module user.module handlers
 *
 * @{
 */

/**
 * Implements hook_views_data()
 */
function filter_jesa_views_data() {

	// uid
  $data['users']['uid_jesa_c'] = array(
    'real field' => 'uid',
	  'group'      => t('User'),
    'title'      => t('Jesa filter'),
    'help'       => t('Filter the view to the currently logged in user except if having one of the specified permissions.'),

  'filter' => array(
      'handler' => 'filter_jesa_handler_user',      
    ),
  );


  return $data;
}

/**
 * Allow replacement of current userid so we can cache these queries
*/ 
function filter_jesa_query_substitutions($view) {
  global $user;
  return array('***CURRENT_USER***' => intval($user->uid));
}

/**
 * @}
 */
