<?php
/**
*  Fonction de rappel (callback) pour la gestion des inscriptions.
* 
* Contient deux formulaires et leurs fonctions associées.
* - Le formulaire d'introduction d'un stagiaire.
* - Le formulaire d'introduction des parents et de leur(s)) enfant(s).
*  @file
*  @author    Jean-Paul Scandariato
*  @date      25/10/2011
*/


/**
 * Constructeur du formulaire de gestion des inscriptions
 *
 */
function inscription_jesa_manage($form, &$form_state) {

}

/**
 * Fonction validation pour  inscription_jesa_manage().
 *
 */
function inscription_jesa_manage_validate($form, &$form_state) {
  
}

/**
 * Fonction d'exécution pour  inscription_jesa_manage().
 *
 */
function inscription_jesa_manage_submit($form, &$form_state) {
  
}


/**
*  Constructeur du formulaire d'introduction des parents et des enfants.
*
*  Le formulaire est prévu pour permettre l'introduction d'un adulte
*  et d'un ou plusieurs enfants (les stagiaires).
*  Il sera utilisé si le parent et/ou l'enfant n'est pas encore connu
*  en tant qu'utilisateur.
*  Ce formulaire controlera si le parent et/ou les enfants existent déjà
*  en tant qu'utilisateur avant de les créer. 
*  
*  @see inscription_jesa_link_validate()
*  @see inscription_jesa_link_submit()
*  @see <a href="http://api.drupal.org/api/drupal/developer--topics--forms_api_reference.html/7">Forms api reference</a>
*  @see <a href="http://drupal.org/node/37775">Form api</a>
*/
function inscription_jesa_link($form, &$form_state) {
  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#type'   => 'item',
    '#title' => t('This page allow you to create the link between a parent and one of more child and, if necessary, create the users.
                  This will allow the parent to register his child for the future events.'
    ),
  );
  $form['stagiaire']['parent'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Parent'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
  $form['stagiaire']['parent']['existant'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Existing parent'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
 $form['stagiaire']['parent']['existant']['user_name'] = array(
    '#title'             => t('Existing parent'),
    '#type'              => 'textfield',
    '#autocomplete_path' => 'inscriptions/jeunes/admin/link/parent_autoc',
   );
 $form['stagiaire']['parent']['nouveau'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('New Parent'),
    '#collapsible' => TRUE,
    '#collapsed'   => TRUE,
  );  

  //Pour les parents, nous n'avons pas besoin de la date de naissance
  $selection = array(
    'nom'            => array('required' => FALSE,),
    'prenom'         => array('required' => FALSE,),
    'date_naissance' => FALSE,
    'mail'           => array('required' => FALSE,),
    'telephone'      => array('required' => FALSE,),
    'adresse_1'      => array('required' => FALSE,),
    'adresse_2'      => array('required' => FALSE,),
    'sexe'           => array('required' => FALSE,),
  );
  $form['stagiaire']['parent']['nouveau'] += _inscription_jesa_get_form_user_fields ($selection);

   
  if (empty($form_state['num_stagiaires'])) {
    $form_state['num_stagiaires'] = 1;
  };

  // Build the number of name fieldsets indicated by $form_state['num_stagiaires']
  for ($i = 1; $i <= $form_state['num_stagiaires']; $i++) {
    $form['stagiaire'][$i] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Participant') . ' #' . $i,
      '#collapsible' => TRUE,
      '#collapsed'   => FALSE,
    );
    $form['stagiaire'][$i]['existant'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('Existing member'),
      '#collapsible' => TRUE,
      '#collapsed'   => FALSE,
    );
   $form['stagiaire'][$i]['existant']['user_name'] = array(
      '#title'             => t('Existing member'),
      '#type'              => 'textfield',
      '#autocomplete_path' => 'inscriptions/jeunes/admin/direct/stagiaire_autoc',
     );
   $form['stagiaire'][$i]['nouveau'] = array(
      '#type'        => 'fieldset',
      '#title'       => t('New member'),
      '#collapsible' => TRUE,
      '#collapsed'   => isset($form_state['values']['stagiaire'][$i]['nouveau']['collapsed']) ? $form_state['values']['stagiaire'][$i]['nouveau']['collapsed'] : TRUE,
    );
    
    //nous ne demandons pas l'adresse pour les enfants, nous prendrons
    // celle du parent.
   $selection = array(
    'nom'            => array('required' => FALSE,),
    'prenom'         => array('required' => FALSE,),
    'date_naissance' => array('required' => FALSE,),
    'mail'           => array('required' => FALSE,),
    'telephone'      => array('required' => FALSE,),
    'adresse_1'      => FALSE,
    'adresse_2'      => FALSE,
    'sexe'           => array('required' => FALSE,),
  );
    
    
    $form['stagiaire'][$i]['nouveau'] += _inscription_jesa_get_form_user_fields ($selection);
    $form['stagiaire'][$i]['lien_parente'] = array(
    '#title'         => t('Kinship'),
    '#type'          => 'select',
    '#options'       => array ('P' => 'Pere', 'M' => 'Mere', 'T'=> 'Tuteur'),
    );
  };
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  // Adds "Add another name" button
  $form['add_child'] = array(
    '#type' => 'submit',
    '#value' => t('Add a new name'),
    '#submit' => array('inscription_jesa_link_add_child'),
  );

  // If we have more than one name, this button allows removal of the
  // last name.
  if ($form_state['num_stagiaires'] > 1) {
    $form['remove_child'] = array(
      '#type' => 'submit',
      '#value' => t('Delete the last name'),
      '#submit' => array('inscription_jesa_link_remove_child'),
      // Since we are removing a name, don't validate until later.
      '#limit_validation_errors' => array(),
    );
  }

  return $form;
}

/**
 * Submit handler for "Add another name" button on inscription_jesa_link().
 *
 * $form_state['num_stagiaires'] tells the form builder function how many name
 * fieldsets to build, so here we increment it.
 *
 * All elements of $form_state are persisted, so there's no need to use a
 * particular key. We can just use $form_state['num_stagiaires'].
 */
function inscription_jesa_link_add_child($form, &$form_state) {
  // Everything in $form_state is persistent, so we'll just use
  // $form_state['add_stagiaire']
  $form_state['num_stagiaires']++;

  // Setting $form_state['rebuild'] = TRUE causes the form to be rebuilt again.
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for "Remove  name" button on inscription_jesa_link().
 *
 * $form_state['num_stagiaires'] tells the form builder function how many name
 * fieldsets to build, so here we decrement it.
 *
 * All elements of $form_state are persisted, so there's no need to use a
 * particular key. We can just use $form_state['num_stagiaires'].
 */
function inscription_jesa_link_remove_child($form, &$form_state) {
  if ($form_state['num_stagiaires'] > 1) {
    $form_state['num_stagiaires']--;
  }

  // Setting $form_state['rebuild'] = TRUE causes the form to be rebuilt again.
  $form_state['rebuild'] = TRUE;
}

/**
 * Fonction de validation pour inscription_jesa_link().
 *
 * Pour le parent: vérifie s'il y a déjà un utilisateur avec le même nom et 
 * prénom, vérifie s'il y a un utilisateur avec la même adresse mail et si 
 * l'utilisateur trouvé par le nom correspondt à celui trouvé par le nom.
 * Si un utilisateur à été trouvé, stocke l'uid pour traitement ultérieur dans 
 * la fonction inscription_jesa_link_submit().
 */
function inscription_jesa_link_validate($form, &$form_state) {
  //**************************
  // Validation du parent
  //**************************
  //Si le champ stagiaire existant est rempli, on ne regade pas les champs nouveau
  if (!empty($form_state['values']['stagiaire']['parent']['existant']['user_name'])) {
    //Controle si le parent existe déjà 
    $parent_account = user_load_by_name($form_state['values']['stagiaire']['parent']['existant']['user_name']);
    if (empty($parent_account)) {
      form_set_error('stagiaire][parent][existant][user_name', 'Le parent n&rsquo;existe pas en tant qu&rsquo;utilisateur. Veuillez le cr&eacute;er en introduisant les informations dans Nouveau Parent.');   
    }
    else {
      $form_state['values']['stagiaire']['parent']['uid'] = $parent_account->uid;      
    }    
  } 
  else {
    // Nouveau Parent, on controle les données
    $parent = _inscription_jesa_get_user_data($form_state['values']['stagiaire']['parent']['nouveau']);
    if (empty($parent['nom'])) {
      form_set_error('stagiaire][parent][nouveau][nom',  t('Parent') . ': ' . t('the name is mandatory for a new member.'));
    }
    if (empty($parent['prenom'])) {
      form_set_error('stagiaire][parent][nouveau][prenom',  t('Parent') . ': ' . t('the firstname is mandatory for a new member.'));
    }
    $parent_account = _inscription_jesa_validate_user($parent);
    //injection de l'identifiant pour utilisation ultérieure dans inscription_jesa_submit
    if (is_string($parent_account)) {
      form_set_error('stagiaire][parent][nouveau', $parent_account);   
    }
    elseif ($parent_account === FALSE) {
      //C'est un nouveau parent (on ne l'a pas trouvé dans la DB) => certains champs deviennent obligatoires
      if (empty($parent['adresse_1']) || empty($parent['adresse_2'])) {
        form_set_error('stagiaire][parent][nouveau', t('Parent') . ': ' . t('the address is mandatory for a new member.'));
        form_set_error('stagiaire][parent][nouveau][adresse_2', ' ');
        //technique pour supprimer le dernier message d'erreur tout en laissant le champ en erreur
        array_pop($_SESSION['messages']['error']);       
      }
      if (empty($parent['sexe'])) {
        form_set_error('stagiaire][parent][nouveau][sexe', t('Parent') . ': ' . t('the gender is mandatory for a new member.'));
      } 
      if (empty($parent['mail']) && empty($parent['nouveau']['telephone'])) {
        form_set_error(
          'stagiaire][parent][nouveau][mail', 
          t('Parent') . ': ' . t('the email address or the phone number is mandatory for a new member.')
        );
        form_set_error('stagiaire][parent][telephone', ' ');
        //technique pour supprimer le dernier message d'erreur tout en laissant le champ en erreur
        array_pop($_SESSION['messages']['error']); 
      }    
    }
    else {
      //renvoie l'utisateur trouvé
      $form_state['values']['stagiaire']['parent']['uid'] = $parent_account->uid;
    }
  }   
     
  //**************************
  //Validation des enfants 
  //**************************
  for ($i = 1; $i <= $form_state['num_stagiaires']; $i++) {
    if (!empty($form_state['values']['stagiaire'][$i]['existant']['user_name'])) {
      //Controle si le stagiaire existe déjà et si les champs sont corrects
      $stagiaire_account = user_load_by_name($form_state['values']['stagiaire'][$i]['existant']['user_name']);
      if (empty($stagiaire_account)) {
        form_set_error('stagiaire][' . $i . '][existant][user_name', 'Participant #' . $i . ' : ' .'Le stagiaire n&rsquo;existe pas en tant qu&rsquo;utilisateur. Veuillez le cr&eacute;er en introduisant les informations dans Nouveau Stagiaire.');   
      } 
      else {
        $form_state['values']['stagiaire'][$i]['existant']['uid']    = $stagiaire_account->uid;    
        $form_state['values']['stagiaire'][$i]['existant']['nom']    = $stagiaire_account->field_nom[LANGUAGE_NONE][0]['value'];
        $form_state['values']['stagiaire'][$i]['existant']['prenom'] = $stagiaire_account->field_prenom[LANGUAGE_NONE][0]['value'];
      }
    } 
    else {
      $stagiaire = _inscription_jesa_get_user_data($form_state['values']['stagiaire'][$i]['nouveau']);    
      // Validité de la date de naissance
      if (empty($stagiaire['date_naissance'])) {
          form_set_error(
            'stagiaire][' . $i . '][nouveau][date_naissance', 
                        t('Participant') . ' #' . $i . ' : ' . t('the birthdate is mandatory for a new member.')
          );
      }
      else {       
        $date_naissance_dt = _inscription_jesa_chek_birthdate($stagiaire['date_naissance']);
        if (is_int($date_naissance_dt)) {
            $form_state['values']['stagiaire'][$i]['nouveau']['date_naissance_ts']  = $date_naissance_dt;
        } 
        else {
          form_set_error('stagiaire][' . $i . '][nouveau][date_naissance', 'Participant #' . $i . ' : ' . $date_naissance_dt);
          unset($form_state['values']['stagiaire'][$i]['nouveau']['date_naissance_ts']);
          }
    }            
   
      if (empty($stagiaire['nom'])) {
        form_set_error(
          'stagiaire][' . $i . '][nouveau][nom', 
          t('Participant') . ' #' . $i . ' : ' . t('the name is mandatory for a new member.')
        );
      }
      if (empty($stagiaire['prenom'])) {
        form_set_error(
          'stagiaire][' . $i . '][nouveau][prenom', 
          t('Participant') . ' #' . $i . ' : ' . t('the firstname is mandatory for a new member.')
        );
      }  
      
      $stagiaire_account = _inscription_jesa_validate_user($stagiaire);
      if (is_string($stagiaire_account)) {
        //erreur durant la validation de l'utilisateur.
        form_set_error('stagiaire][' . $i, $stagiaire_account);   
      } 
      elseif ($stagiaire_account === FALSE) {
        //C'est un nouveau stagiaire => certains champs deviennent obligatoires
        if (empty($stagiaire['sexe'])) {
          form_set_error(
            'stagiaire][' . $i . '][nouveau][sexe',  
            t('Participant') . ' #' . $i . ' : ' . t('the gender is mandatory for a new member.')
          );
        } 
        if (empty($stagiaire['mail']) && empty($stagiaire['telephone'])) {
          form_set_error(
            'stagiaire][' . $i . '][nouveau][mail',  
            'Participant #' . $i . ' : ' . t('the email address or the phone number is mandatory for a new member.')
          );
          form_set_error('stagiaire][' . $i . '][nouveau][telephone', ' ');
          array_pop($_SESSION['messages']['error']); 
        }    
      } 
      else {
        //renvoie l'utisateur trouvé
        $form_state['values']['stagiaire'][$i]['existant']['uid'] = $stagiaire_account->uid;
        $form_state['values']['stagiaire'][$i]['existant']['nom'] = $stagiaire_account->field_nom[LANGUAGE_NONE][0]['value'];
        $form_state['values']['stagiaire'][$i]['existant']['prenom'] = $stagiaire_account->field_prenom[LANGUAGE_NONE][0]['value'];
      }
    }
   
    // A-t'on toutes les données pour créer le lien ?
    if (is_object($stagiaire_account) && is_object($parent_account)) {
      //Des utilisateurs sont définis pour le parent et pour le stagiaire,
      // on peut contrôler si le lien existe et est correct.
  	  $stagiaire['uid']    = $stagiaire_account->uid;
      $stagiaire['nom']    = $stagiaire_account->field_nom[LANGUAGE_NONE][0]['value'];
      $stagiaire['prenom'] = $stagiaire_account->field_prenom[LANGUAGE_NONE][0]['value'];
  	  $parent['uid']       = $parent_account->uid;
  	  $action_lien = _inscription_jesa_check_lien($stagiaire, $parent, $form_state['values']['stagiaire'][$i]['lien_parente']);
    }            
  	else {
      //Au moins une des deux personnes n'est pas définie => dans tous les cas il faudra
      // créer le lien.
      $action_lien = 'create';
    }
 
    
    if ($action_lien === 'create') {
      $form_state['values']['stagiaire'][$i]['parent_action'] = 'create';
    }      
    elseif ($action_lien === TRUE) {
      // Le lien existe déjà => ne rien faire
      $form_state['values']['stagiaire'][$i]['parent_action'] = '';
    }
    else {
      // si la fonction _inscription_jesa_check_lien est erronnée elle renvera un message d'erreur dans la valeur de retour.
      form_set_error('stagiaire][' . $i ,'Participant #' . $i . ' : ' . $action_lien);
    }
  }
}

/**
 * Fonction d'exécution pour inscription_jesa_link().
 *
 * Traite les données du formulaire s'il passe correctement la fonction de
 * validation inscription_jesa_validate()
 */
function inscription_jesa_link_submit($form, &$form_state) {
  global $user;
  //création du parent
  $parent = _inscription_jesa_get_user_data($form_state['values']['stagiaire']['parent']);
  if (empty($parent['uid'])) {
    try {
      $account_parent = _inscription_jesa_create_parent($parent['nouveau']);
      $parent['uid']  = $account_parent->uid;
      drupal_set_message(
        t('@firstname @name, user successfuly created.', 
        array('@name' => $parent['nom'], '@firstname' => $parent['prenom'],))
      );        
    }
    catch (ErrorException $e) {
      drupal_set_message(
        t('Error during the processing of @prenom @nom (!msg).',
          array('@nom' => $stagiaire['nom'], '@prenom' => $stagiaire['prenom'], '!msg' =>$e->getMessage(),)
        ),
        'error'
      );
      return;
    }
  }
  else {
    $account_parent = user_load($parent['uid']);
    //attribution du rôle parent à l'utilisateur (s'il ne l'a pas déjà)
    _inscription_jesa_add_role_to_user($account_parent, 'Parent');
  }

  
  
  for ($i = 1; $i <= $form_state['num_stagiaires']; $i++) {
    try {
      $stagiaire = $form_state['values']['stagiaire'][$i]['existant'];
      if (!empty($stagiaire['uid'])) {
        // Soit on a introduit des données dans les champs "nouveau stagiaire" pour un stagiaire existant 
        // soit on a utilisé le champs stagiaire existant
        //  Dans ces deux cas on ne crée pas le stagiaire mais on utilise les données existantes
        if(empty($form_state['values']['stagiaire'][$i]['existant']['user_name'])) {
          //on a introduit des données dans les champs "nouveau stagiaire" pour un stagiaire existant
          // envoi d'un message d'avertissement.
          drupal_set_message('The participant already exists, he will not be created.', 'Warning');  
        }
        // doit-on créer le lien parent?
        if ($form_state['values']['stagiaire'][$i]['parent_action'] == 'create') {
          _inscription_jesa_create_lien($stagiaire, $account_parent, $form_state['values']['stagiaire'][$i]['lien_parente']);
          drupal_set_message(
            t('Participant') . ' #' . $i . ' : ' . 
            t('@firstname @name, link successfuly activated.', 
                array('@name' => $stagiaire['nom'], '@firstname' => $stagiaire['prenom'],))
           );
        }
        else {
            drupal_set_message(
              t('Participant') . ' #' . $i . ' : ' . 
              t('@firstname @name, the user and the link already exists.', 
              array('@name' => $stagiaire['nom'], '@firstname' => $stagiaire['prenom'],))
            );        
        }
      }
      else {
        //Le stagiaire n'existe pas => créer le stagiaire et le lien
        // On place l'addresse du parent chez le stagiaire      
        $stagiaire = _inscription_jesa_get_user_data($form_state['values']['stagiaire'][$i]['nouveau']);
        $stagiaire['adresse_1'] = $account_parent->field_adresse_1[LANGUAGE_NONE][0]['value'];
        $stagiaire['adresse_2'] = $account_parent->field_adresse_2[LANGUAGE_NONE][0]['value'];
        $account_stagiaire = _inscription_jesa_create_user_stagiaire($stagiaire);
        $stagiaire['uid'] = $account_stagiaire->uid;
        _inscription_jesa_create_lien($stagiaire, $account_parent, $form_state['values']['stagiaire'][$i]['lien_parente']);
        drupal_set_message(
          t('Participant') . ' : ' . $i . ' : ' . 
          t('user and link successfully created.', 
            array('@nom' => $stagiaire['nom'], '@prenom' => $stagiaire['prenom'],))
        );
      }
    }
    catch (ErrorException $e) {
    drupal_set_message(
      t('Error during the processing of @prenom @nom (!msg).',
        array('@nom' => $stagiaire['nom'], '@prenom' => $stagiaire['prenom'],'!msg' =>$e->getMessage(),)
      ),
      'error'
      );
    }
  }
}

/**
 * Constructeur du formulaire d'introduction d'un stagiaire
 *
 */
function inscription_jesa_direct($form, &$form_state) {
  $form['#tree'] = TRUE;

  $form['description'] = array(
    '#type'   => 'item',
    '#title' => t('Allow to register a new participant. if the participant does\'t exist he(she) will be created. For a new member you must provide the name, firstname, the gender and a contact method (mail or phone).'),
  );
  $form['stagiaire'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Member'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
  $form['stagiaire']['existant'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Existing member'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
 $form['stagiaire']['existant']['user_name'] = array(
    '#title'             => t('Existing member'),
    '#type'              => 'textfield',
    '#autocomplete_path' => 'inscriptions/jeunes/admin/direct/stagiaire_autoc',
   );
 $form['stagiaire']['nouveau'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('New member'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
  $selection = array(
    'nom'            => array('required' => FALSE,),
    'prenom'         => array('required' => FALSE,),
    'date_naissance' => array('required' => FALSE,),
    'mail'           => array('required' => FALSE,),
    'telephone'      => array('required' => FALSE,),
    'adresse_1'      => array('required' => FALSE,),
    'adresse_2'      => array('required' => FALSE,),
    'sexe'           => array('required' => FALSE,),
  );
  $form['stagiaire']['nouveau'] += _inscription_jesa_get_form_user_fields($selection);
  $form['stage'] = array(
    '#type'        => 'fieldset',
    '#title'       => t('Events'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
  );
  //on permet au gestionaire de stage d'inscrire un stagiaire juqu'à 10 jours après
  // le début du stage.
 $form['stage']['list'] = array(
    '#type'    => 'radios',
    '#options' => _inscription_jesa_get_next_stages(4, -10),
    '#title'   => t('Events'),
  );
  $selection = array(
    'train' => array('required' => FALSE,),
  );
  $form['stage'] += _inscription_jesa_get_form_incscription_fields($selection);
  $form['stage']['formulaire'] = array(
      '#type'      => 'checkbox',
      '#title'     => t('Form'),
      '#description' => t('Is the registration form already received.'),
      '#return_value' => 1, 
      '#default_value' => 0, 
      '#required'  => FALSE,
  ); 
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );  

  return $form;
}

/**
 * Fonction validation pour  inscription_jesa_direct().
 *
 */
function inscription_jesa_direct_validate($form, &$form_state) {
  //**************************
  //  Validation Utilisateur 
  //**************************
  //Si le champ stagiaire existant est rempli, on ne regade pas les champs nouveau
  if (!empty($form_state['values']['stagiaire']['existant']['user_name'])) {
    //Controle si le stagiaire existe déjà et si les champs sont corrects
    $user_by_name = user_load_by_name($form_state['values']['stagiaire']['existant']['user_name']);
    if (empty($user_by_name)) {
      form_set_error('stagiaire][existant][user_name', 'The member doesn\'t exist.');   
    }
    $form_state['values']['stagiaire']['uid'] = is_object($user_by_name) ? $user_by_name->uid : 0;
  } 
  else {
    // Le champs utilisateur existant n'a pas été rempli, on controle les champs
    // du nouvel utilisateur
    $stagiaire = _inscription_jesa_get_user_data($form_state['values']['stagiaire']['nouveau']);
    // Validité de la date de naissance
    if (empty($stagiaire['date_naissance'])) {
        form_set_error('stagiaire][nouveau][date_naissance', ucfirst(t('the birthdate is mandatory for a new member.')));
    }
    else {      
      $date_naissance_dt = _inscription_jesa_chek_birthdate($stagiaire['date_naissance']);
      if (is_int($date_naissance_dt)) {
          $form_state['values']['stagiaire']['nouveau']['date_naissance_ts']  = $date_naissance_dt;
      } 
      else {
        form_set_error('stagiaire][nouveau][date_naissance', $date_naissance_dt);
        unset($form_state['values']['stagiaire']['nouveau']['date_naissance_ts']);
      }
    }

    if (empty($stagiaire['nom'])) {
      form_set_error('stagiaire][nouveau][nom', ucfirst(t('the name is mandatory for a new member.')));
    }
    if (empty($stagiaire['prenom'])) {
      form_set_error('stagiaire][nouveau][prenom', ucfirst(t('the fisrtname is mandatory for a new member.')));
    }
    //Controle si le stagiaire existe déjà et si les champs sont corrects
    $stagiaire_account = _inscription_jesa_validate_user($stagiaire);
    if (is_string($stagiaire_account)) {
      // le résultat est un string => c'est un message d'erreur.
      form_set_error('stagiaire][nouveau', $stagiaire_account);   
    }
    $form_state['values']['stagiaire']['uid'] = is_object($stagiaire_account) ? $stagiaire_account->uid : 0;
    if ($stagiaire_account === FALSE) {
      //C'est un nouveau stagiaire => certains champs deviennent obligatoires
      if (empty($stagiaire['adresse_1']) || empty($stagiaire['adresse_2'])) {
        form_set_error('stagiaire][nouveau', ucfirst(t('the address is mandatory for a new member.')));
        form_set_error('stagiaire][nouveau][adresse_2', ' ');
        array_pop($_SESSION['messages']['error']);       
      }
      if (empty($stagiaire['sexe'])) {
        form_set_error('stagiaire][nouveau][sexe', ucfirst(t('the gender is mandatory for a new member.')));
      } 
      if (empty($stagiaire['mail']) && empty($stagiaire['telephone'])) {
        form_set_error(
          'stagiaire][nouveau][mail', 
          ucfirst(t('the email address or the phone number is mandatory for a new member.'))
        );
        form_set_error('stagiaire][telephone', ' ');
        array_pop($_SESSION['messages']['error']); 
      }    
    }    
  }
  
  //**************************
  //  Validation Stage
  //**************************
  if (empty($form_state['values']['stage']['list'])) {
    form_set_error('stage][list' ,t('Please select an event.'));
  }
}

/**
 * Fonction d'exécution pour  inscription_jesa_direct().
 *
 */
function inscription_jesa_direct_submit($form, &$form_state) {
  try {
    $stagiaire['uid'] = $form_state['values']['stagiaire']['uid'];
    if ($stagiaire['uid']) {
      if(empty($form_state['values']['stagiaire']['existant']['user_name'])) {
        drupal_set_message(t('The member already exists it will not be created.'), 'Warning');  
      }
    }
    else {
      $stagiaire = _inscription_jesa_get_user_data($form_state['values']['stagiaire']['nouveau']);
      $account_stagiaire = _inscription_jesa_create_user_stagiaire($stagiaire);
      $stagiaire['uid'] = $account_stagiaire->uid;
    }

    _inscription_jesa_set_incription(
      $stagiaire['uid'], 
      $form_state['values']['stage']['list'], 
      $form_state['values']['stage']['train'], 
      $form_state['values']['stage']['annee'],
      $form_state['values']['stage']['formulaire']
    );
  }
  catch (ErrorException $e) {
    drupal_set_message(
      t('Error during the processing of @prenom @nom (!msg).',
        array('@nom' => $stagiaire['nom'], '@prenom' => $stagiaire['prenom'], '!msg' =>$e->getMessage(),)
      ),
      'error'
    );
  }
}

/**
*  Change le statut du formulaire (dans le node stagiaire)
* 
* Bascule le statut du champs formulaire afin de permettre 
* au gestionnaire de stage d'indiqué lorsque le formulaire
* a été reçu.
* Est utilisé dans la vue Stagiaires - par stage (Staff)
* 
* @param $node_stagiaire_nid nid Identification du node d'inscription au stage.
* 
*/
function inscription_jesa_toggle_formulaire($node_stagiaire_nid) {
  $node_stagiaire = node_load($node_stagiaire_nid);
    // XOR the current status with 1 to get the opposite value.
  $node_stagiaire->field_stagiaire_formulaire[LANGUAGE_NONE][0]['value'] = $node_stagiaire->field_stagiaire_formulaire[LANGUAGE_NONE][0]['value'] ^ 1;
  // If this content type specifies that a new revision should be created on
  // editing, then make sure to respect this option.
  $node_options = variable_get('node_options_' . $node_stagiaire->type, array());
  if (in_array('revision', $node_options)) {
    $node_stagiaire->revision = 1;
  }
  node_save($node_stagiaire);
  drupal_set_message(t('The status of the registration form has been modified.'),'status');
  drupal_goto($_SERVER['HTTP_REFERER']);
}

/**
*  Change le statut du paiement (dans le node stagiaire)
* 
* Bascule le statut du champs paiement afin de permettre 
* au gestionnaire de stage d'indiqué lorsque le paiement
* a été reçu.
* Est utilisé dans la vue Stagiaires - par stage (Staff)
* 
* @param $node_stagiaire_nid nid Identification du node d'inscription au stage.
* 
*/
function inscription_jesa_toggle_paiement($node_stagiaire_nid) {
  $node_stagiaire = node_load($node_stagiaire_nid);
    // XOR the current status with 1 to get the opposite value.
  $node_stagiaire->field_stagiaire_paiement[LANGUAGE_NONE][0]['value'] = $node_stagiaire->field_stagiaire_paiement[LANGUAGE_NONE][0]['value'] ^ 1;
  // If this content type specifies that a new revision should be created on
  // editing, then make sure to respect this option.
  $node_options = variable_get('node_options_' . $node_stagiaire->type, array());
  if (in_array('revision', $node_options)) {
    $node_stagiaire->revision = 1;
  }
  node_save($node_stagiaire);
  drupal_set_message(t('The status of the payment has been modified.'),'status');
  drupal_goto($_SERVER['HTTP_REFERER']);

}



/**
* Fonction d'auto-completion pour le champ stagiaire.
*
* Renvoie la liste des stagiaires qui contient les caractères
* introduits par l'utilisateur quelque part dans leur nom ou prénom
*
* @param $string string. Contient les caractères à rechercher.
*
* @return un tableau contenant comme clef le nom d'utilisateur des
*   des stagiaires trouvés et comme valeur la concaténation de leur
*   prénom et nom.
* 
*/
function _inscription_jesa_stagiaire_autocomplete($string) {
  $stagiaires = _inscription_jesa_get_users_with_role($string, 'Stagiaire');
  $matches = array();
  foreach ($stagiaires as $stagiaire) {
    $matches[$stagiaire->name] = $stagiaire->prenom . ' ' . $stagiaire->nom;
  }
   // return for JS
  drupal_json_output($matches);  
}

/**
* Fonction d'auto-completion pour le champ parent.
*
* Renvoie la liste des parents qui contient les caractères
* introduits par l'utilisateur quelque part dans leur nom ou prénom
*
* @param $string string. Contient les caractères à rechercher.
*
* @return un tableau contenant comme clef le nom d'utilisateur des
*   des parents trouvés et comme valeur la concaténation de leur
*   prénom et nom.
* 
*/
function _inscription_jesa_parent_autocomplete($string) {
  $parents = _inscription_jesa_get_users_with_role($string, 'Parent');
  $matches = array();
  foreach ($parents as $parent) {
    $matches[$parent->name] = $parent->prenom . ' ' . $parent->nom;
  }
   // return for JS
  drupal_json_output($matches);  
}

/**
* Contrôle de l'identité de la personne.
*
* Vérifie si la personne existe en tant qu'utilisateur. En recherchant 
*  au travers de son adresse mail et de son nom et prénom.
* Si l'utilisateur est trouvé par les deux méthodes, vérifie s'il s'agit bien
* du même utilisateur.
* Si l'utilisateur est uniquement trouvé par son adresse mail, vérifie si son
* nom et prénom correspondent à son nom d'utilisateur.
*
* @param $user_data array. Contient les informations introduites dans le
*                   formulaire.
*
* @return l'objet utilisateur si celui-ci est trouvé.@n
*  (bool)False si l'utilisateur n'est pas trouvé.@n
*   Un message d'erreur dans les autres cas.
* 
*/
function _inscription_jesa_validate_user($user_data) {
  $return = FALSE;
  if (!empty($user_data['mail']) && !valid_email_address($user_data['mail'])) {
    $return = t('The email address is not valid.');    
  }
  else {
    $user_by_name = user_load_by_name($user_data['user_name']);
    $user_by_mail = user_load_by_mail($user_data['mail']);
  
    if (empty($user_by_name->uid) && empty($user_by_mail->uid)) {
    //L'utilisateur n'a pas été trouvé ni par son adresse mail ni par son nom
    // validation correcte => on va créer l'utilisateur dans submit
    }      
    elseif (!empty($user_by_name->uid) && empty($user_by_mail->uid)) {
    // l'utilisateur a été trouvé par son nom et comme il n'y a pas
    // d'email il n'y a pas de conflit, on utilise la référence trouvée
      $return = $user_by_name;
    }            
    elseif (empty($user_by_name->uid) && !empty($user_by_mail->uid)) {
      // l'utilisateur a été trouvé par son mail et il n'y a pas
      // été trouvé par son nom => pas normal probablement une adresse mail
      // déjà utilisée 
      $return = t('The email address (@email) is already used for another user (@firstname @name).',
        array(
          '@firstname' => $user_by_mail->field_prenom[LANGUAGE_NONE][0]['value'],
          '@name'      => $user_by_mail->field_nom[LANGUAGE_NONE][0]['value'],
          '@email'     => $user_data['mail'],
        )
      );    
    }                      
    else {
      //Les deux références ont donné un utilisateur
      // on controle que les deux utilisateurs sont identiques
      if ($user_by_name->uid == $user_by_mail->uid) {
      //les deux utilisateurs sont les mêmes
      $return = $user_by_name; 
    }      
      else {
      //problème, les deux utilisateurs trouvés ne correspondent pas
      $return = t('The user found using the name (@nom) is different from the one found using the email address (@email).',
        array(
          '@nom'   => $user_by_name->name,
          '@email' => $user_by_mail->name,
        )
      );
    }                   
    }                                
  } 

  return $return;
}

/**
* Crée un tableau associatif contenant une personne introduite dans le formulaire.
*
* Le tableau va être rempli en fonction des données transmises dans le 
* formulaire sur la personne (parent ou enfant) à traiter.
* 
* @return Un tableau associatif contenant
*   - @c uid : l'identification de l'utilisateur (si existant), 0 autrement.
*   - @c mail : son adrres email (si introduit), '' autrement.
*   - @c nom : son nom.
*   - @c prenom : son prénom.
*   - @c sexe : son sexe.
*   - @c telephone : Son numéro de téléphone.
*   - @c adresse_1 : Pour le parent, la première ligne de l'adresse. Pour l'enfant une chaîne vide.
*   - @c adresse_2 : Pour le parent, la seconde ligne de l'adresse. Pour l'enfant une chaîne vide.
*   - @c date_naissance : Pour l'enfant, sa date de naissance. Pour le parent la valeur NULL.
**/
function _inscription_jesa_get_user_data($form_info) {
  $nom = isset($form_info['nom']) ? $form_info['nom'] : '';
  $prenom = isset($form_info['prenom']) ? $form_info['prenom'] : '';
  return   array(
    'uid'            => isset($form_info['uid']) ? $form_info['uid'] : 0,
    'mail'           => isset($form_info['mail']) ? $form_info['mail'] : '',
    'nom'            => $nom,
    'prenom'         => $prenom,
    'user_name'      => _inscription_jesa_generate_user_name($nom, $prenom),
    'adresse_1'      => isset($form_info['adresse_1']) ? $form_info['adresse_1'] : '',
    'adresse_2'      => isset($form_info['adresse_2']) ? $form_info['adresse_2'] : '',
    'sexe'           => isset($form_info['sexe']) ? $form_info['sexe'] : 'M',
    'telephone'      => isset($form_info['telephone']) ? $form_info['telephone'] : NULL,
    'date_naissance_ts' => isset($form_info['date_naissance_ts']) ? $form_info['date_naissance_ts'] : NULL,
    'date_naissance' => isset($form_info['date_naissance']) ? $form_info['date_naissance'] : NULL,
  );
}

/**
*  Création de l'utilisateur parent
* 
* Si un problème se produit lors de la création une exception 
* de type ErrorException sera lancée
*   
* @return l'objet utilisateur en cas de succès.
*/
function _inscription_jesa_create_parent($parent) {
  return _inscription_jesa_create_user($parent, array(8 => TRUE));
}

/**
*  Création de l'utilisateur stagiaire
* 
* Si un problème se produit lors de la création une exception 
* de type ErrorException sera lancée
*   
* @return l'objet utilisateur en cas de succès.
*/
function _inscription_jesa_create_user_stagiaire($stagiaire) {
  // crée un utilisateur avec le role de stagiaire (4)
  return _inscription_jesa_create_user($stagiaire, array(4 => TRUE));
}

/**
* Crée le nom d'utilisateur en fonction du prénom et du nom transmis.
* 
* Le nom d'utilisateur est construit sous la forme "prénom nom" avec la 
* première lettre de chaque partie en majuscule. 
* 
* @return Le nom d'utilisateur.
**/
function _inscription_jesa_generate_user_name($nom, $prenom) {
  $result = '';
  if (!empty($nom) || !empty($prenom)) {
    $result = mb_convert_case($prenom, MB_CASE_TITLE, 'UTF-8'). ' ' . ucwords(substr($nom, 0, 1)) . substr($nom,strlen($nom)-1,1) . '.';
  }
  return $result;
}

/**
 * Cette fonction génère une adresse email fictive
 * 
 * Les email fictifs sont de la forme xYYYYY@example.com où YYYYY est un nombre.
 * La procédure va chercher la dernière adresse générée de ce type et va 
 * en créer une avec le nombre incrémenté d'une unité.
 * Le domaine example.com ne sera jamais exploité donc pas de problème
 * cf RFC2606
 * 
 * @warning L'adresse mail retournée est générée à partir de la dernière adresse 
 * trouvée dans la base de données. La fonction renvera toujours la même adresse
 *  tant qu'aucun nouvel utilisateur avec la nouvelle adresse n'est inséré dans
 * la base de donnée.
 *
 * @return La prochaine adresse mail disponible.
 **/
function _inscription_jesa_generate_email() {
  //on va directement produire la nouvelle adresse mail
	$query = 'SELECT CONCAT(\'x\', CONVERT(SUBSTR(mail, 2,LENGTH(mail) - 13) + 1, DECIMAL), \'@example.com\') email
            FROM {users}
            WHERE
              mail REGEXP :reg_test
            ORDER BY
              CONVERT(SUBSTR(mail, 2,LENGTH(mail) - 13) + 1, DECIMAL) DESC
          ';
	try {
    $results = db_query_range($query, 0, 1, array(':reg_test' => '^x[0-9]{1,}@example.com$'), array('throw_exception' => TRUE,));
  }
  catch(PDOException $e) {
    watchdog_exception('jesa inscription', $e, 'DB error of %type: !message in %function (line %line of %file).', array(), WATCHDOG_ALERT, NULL);
    throw new ErrorException(
      t('Error during database access.'),
      0,
      E_USER_NOTICE
    );
  }

  if ($results->rowCount() == 0) {
    // On crée le nouvel utilisateur
    $email = 'x1@example.com';
  }
  elseif ($results->rowCount() == 1) {
    $email = $results->fetchField(0);
  }
  drupal_set_message('email generated : ' . $email);
  return $email;
}

/**
*  Création d'un utilisateur
*
*  Lance une exception ErrorException si l'adresse email est utilisée
*   par un autre utilisateur
**/
function _inscription_jesa_create_user($personne, $roles = array()) {
  if (empty($personne['mail'])) {
    $personne['mail'] = _inscription_jesa_generate_email();    
  }

  //controle que l'identifiant (email) n'est pas déjà utilisé
  if (user_load_by_mail($personne['mail']) == FALSE) {
    
    $account = new stdClass;
    $account->is_new   = TRUE;
    $account->name     = $personne['user_name'];
    $account->mail     = $personne['mail'];
    $account->init     = $personne['mail'];
    $account->status   = TRUE;
    $account->roles    = $roles;
    $account->timezone = variable_get('date_default_timezone', '');
    
    //on protège et la version originale et la safe_value (contre les XSS)
    $prenom = check_plain($personne['prenom']);
    $account->field_prenom[LANGUAGE_NONE][0]['value']      = $prenom;
    $account->field_prenom[LANGUAGE_NONE][0]['safe_value'] = $prenom;
    //on protège et la version originale et la safe_value (contre les XSS)
    $nom = check_plain($personne['nom']);
    $account->field_nom[LANGUAGE_NONE][0]['value']      = $nom;
    $account->field_nom[LANGUAGE_NONE][0]['safe_value'] = $nom;

    $account->field_sexe[LANGUAGE_NONE][0]['value'] = $personne['sexe'];

    if (!empty($personne['adresse_1'])) {
      $adresse = check_plain($personne['adresse_1']);
      $account->field_adresse_1[LANGUAGE_NONE][0]['value']      = $adresse;
      $account->field_adresse_1[LANGUAGE_NONE][0]['safe_value'] = $adresse;
    }
    if (!empty($personne['adresse_2'])) {
      $adresse = check_plain($personne['adresse_2']);
      $account->field_adresse_2[LANGUAGE_NONE][0]['value']      = $adresse;
      $account->field_adresse_2[LANGUAGE_NONE][0]['safe_value'] = $adresse;
    }

    if (!empty($personne['telephone'])) {
      $tel = check_plain($personne['telephone']);
      $account->field_tel[LANGUAGE_NONE][0]['value']      = $tel;
      $account->field_tel[LANGUAGE_NONE][0]['safe_value'] = $tel;
    }
    
    if (!empty($personne['date_naissance_ts'])) {
      $account->field_date_naissance[LANGUAGE_NONE][0]['value'] = date('Y-m-d\TH:i:s', $personne['date_naissance_ts']);
    }

    
    // on va impersonaliser l'utilisateur connecté (qui n'a pas le droit
    //  de créer ou modifier un utilisateur).
    // Cette technique est sécurisée.
    // @todo créer un utilisateur technique pour diminuer les droits associés
    global $user;
    $original_user = $user;
    $old_state = drupal_save_session();
    drupal_save_session(FALSE);
    $user = user_load(1);
    //création du compte
    $result = user_save($account);
    //retour à l'utilisateur normal
    $user = $original_user;
    drupal_save_session($old_state);

    if ($result === FALSE) {
      // on remonte l'erreur
      throw new ErrorException(
        'L&rsquo;utilisateur ne peut pas &ecirc;tre cr&eacute;&eacute;',
        0,
        E_USER_WARNING
      );
    }
  }
  else {
    // on remonte l'erreur
    throw new ErrorException(
      t('The email address @email is already used.', 
        array('@email' => $personne['mail'])),
      0,
      E_USER_NOTICE
    );
  }
  return $result;
}

/**
* Crée le lien entre le parent et le stagiaire.
* Le lien est un node de type parent-enfant (qui ne doit pas être plublié).
* Lance une exception de type ErrorException en cas de problème durant 
* la création du lien.
*
*  @param array $stagiaire Tableau associatif contenant l'identifiant de 
*    du stagiaire dans l'élément 'uid'.
*  @param user $account_parent Objet user du parent.
*  @param $lien 
*    Contient le type de lien entre le parent et le stagiaire.
*    -Les valeurs possibles pour le lien sont:
*       -# P : Père
*       -# M : Mère
*       -# T : Tuteur
**/
function _inscription_jesa_create_lien($stagiaire, $account_parent, $lien) {
  global $user;
  $node = new stdClass();

  $node->type   = 'parent_stagiaire';
  $node->is_new = TRUE;
  node_object_prepare($node);

  $node->name     = $user->name;
  $node->title    = $account_parent->name . ' - ' . $stagiaire['user_name'] . ' - ' . $lien;
  $node->language = 'fr';

  $node->comment  = 0;   // 2 = comments on, 1 = comments off
  $node->status   = NODE_NOT_PUBLISHED;
  $node->promote  = NODE_NOT_PROMOTED;
  $node->revision = 0;
  $node->sticky   = NODE_NOT_STICKY;

  
  $node->field_lien_parent[LANGUAGE_NONE][0]['uid'] = $account_parent->uid;
  $node->field_lien_enfant[LANGUAGE_NONE][0]['uid'] = $stagiaire['uid'];
  $node->field_lien_type[LANGUAGE_NONE][0]['value'] = $lien;
  node_submit($node);
 try {
     // on va impersonaliser l'utilisateur connecté (qui n'a pas le droit
    //  de créer ou modifier un utilisateur).
    // Cette technique est sécurisée.
    // @todo créer un utilisateur technique pour diminuer les droits associés
    global $user;
    $original_user = $user;
    $old_state = drupal_save_session();
    drupal_save_session(FALSE);
    $user = user_load(1);
    //création du compte
    $result = node_save($node);
    //retour à l'utilisateur normal
    $user = $original_user;
    drupal_save_session($old_state);    
  }
  catch (Exception $e) {
    throw new ErrorException(
      'The link cannot be created',
      0,
      E_USER_WARNING
    );
  }
}

/**
* Ajoute un rôle à un utilisateur (si l'utilsateur ne l'a pas déjà)
*
* @param $account
*   account object or user ID.
* @param $role_name
*   String value of role to be added.
*
*/
function _inscription_jesa_add_role_to_user($account, $role_name) {
  // For convenience, we'll allow user ids as well as full user objects.
  if (is_numeric($account)) {
    $account = user_load($account);
  }
  // If the user doesn't already have the role, add the role to that user.
  $key = array_search($role_name, $account->roles);
  if ($key == FALSE) {
    // Get the rid from the roles table.
    $roles = user_roles(TRUE);
    $rid = array_search($role_name, $roles);
    if ($rid != FALSE) {
      $new_role[$rid] = $role_name;
      $all_roles = $account->roles + $new_role; // Add new role to existing roles.
      
      // on va impersonaliser l'utilisateur connecté (qui n'a pas le droit
      //  de créer ou modifier un utilisateur).
      // Cette technique est sécurisée.
      // @todo créer un utilisateur technique pour diminuer les droits associés
      global $user;
      $original_user = $user;
      $old_state = drupal_save_session(FALSE);
      $user = user_load(1);
      //Modification du compte
      $result = user_save($account, array('roles' => $all_roles));
      //retour à l'utilisateur normal
      $user = $original_user;
      drupal_save_session($old_state);         
      
      
    }
  }
}

/**
*  Renvoie l'inscription du prochain stage et son statut
*
*/
function _inscription_jesa_prochain_stage($account_uid) {
  $query = '
    SELECT
      IFNULL(p.field_stagiaire_paiement_value, 0)  AS paiement,
      IFNULL(f.field_stagiaire_formulaire_value, 0) AS formulaire,
      n.title
    FROM
           field_data_field_stagiaire_user u
      JOIN field_data_field_stagiaire_stage s
        ON    s.entity_type = u.entity_type
          AND s.bundle      = u.bundle
          AND s.revision_id = u.revision_id
          AND s.delta       = 0
          AND s.deleted     = 0
          AND s.entity_id   = u.entity_id
      JOIN field_data_field_stage_dates d
        ON    d.entity_type = s.entity_type
          AND d.bundle      = :stage_bundle
          AND d.revision_id = s.field_stagiaire_stage_nid
          AND d.delta       = 0
          AND d.deleted     = 0
          AND d.entity_id   = s.field_stagiaire_stage_nid
      JOIN node n
        ON n.nid = s.field_stagiaire_stage_nid
      JOIN field_data_field_stagiaire_paiement  p
        ON    p.entity_type = u.entity_type
          AND p.bundle      = u.bundle
          AND p.revision_id = u.revision_id
          AND p.delta       = 0
          AND p.deleted     = 0
          AND p.entity_id   = u.entity_id
      JOIN field_data_field_stagiaire_formulaire f
        ON    f.entity_type = u.entity_type
          AND f.bundle      = u.bundle
          AND f.revision_id = u.revision_id
          AND f.delta       = 0
          AND f.deleted     = 0
          AND f.entity_id   = u.entity_id
    WHERE
        u.entity_type = :entity_type
    AND u.bundle      = :stagiaire_bundle
    AND u.deleted     = 0
    AND u.language    = :language
    AND u.delta       = 0
    AND u.field_stagiaire_user_uid   = :uid
    AND d.field_stage_dates_value2 >= DATE_FORMAT(now(),:format_date)
  ';

	try {
    $results = db_query_range(
      $query,
      0,
      1,
      array(
        ':entity_type'      => 'node',
        ':stage_bundle'     => 'stage',
        ':stagiaire_bundle' => 'stagiaire',
        ':language'         => LANGUAGE_NONE,  //language indépendant
        ':uid'              => $user->uid,     // pour l'utilisateur connecté
        ':format_date'      => '%Y-%m-%dT00:00:00', //dans le futur
      ),
      array('throw_exception' => TRUE,));
  }
  catch(PDOException $e) {
    watchdog_exception('jesa inscription', $e, 'DB error of %type: !message in %function (line %line of %file).', array(), WATCHDOG_ALERT, NULL);
    throw new ErrorException(
      t('Error during database access.'),
      0,
      E_USER_NOTICE
    );
  }
  return $results->fetchAll();
}

/**
 *  Génère les champs nécessaires à l'introduction d'un utilisateur.
 *
 *  Par défaut, tous les champs seront utilisés et obligatoires. Le paramètre
 *  $selection permet de modifier ce comportement.
 *  @param array $selection Tableau associatif ayant pour clef le nom du champ,
 *    et pour valeur :
 *    - (champ => FALSE) indique que le champ ne doit pas être utilisé
 *    - (champ => array('required' => TRUE | FALSE) indique que le champ doit 
 *      être utilisé et la valeur de l'option #required.
 **/
 function _inscription_jesa_get_form_user_fields($selection) {
  $return = array();
  if (!isset($selection['prenom']) || ($selection['prenom'] !== FALSE)) {
    $return['prenom'] = array(
      '#type'      => 'textfield',
      '#title'     => t('Firstname'),
      '#size'      => 30,
      '#maxlength' => 30,
      '#required'  => isset($selection['prenom']['required']) ? $selection['prenom']['required'] : TRUE,
    );
  }
  if (!isset($selection['nom']) || ($selection['nom'] !== FALSE)) {
    $return['nom'] = array(
      '#type'      => 'textfield',
      '#title'     => t('Name'),
      '#size'      => 30,
      '#maxlength' => 30,
      '#required'  => isset($selection['nom']['required']) ? $selection['nom']['required'] : TRUE,
    );  
  }
  if (!isset($selection['sexe']) || ($selection['sexe'] !== FALSE)) {
    $return['sexe'] = array(
      '#type'     => 'select',
      '#options'  => array ('M' => 'Masculin', 'F' => 'F&eacute;minin'),
      '#title'    => t('Gender'),
      '#required'  => isset($selection['sexe']['required']) ? $selection['sexe']['required'] : TRUE,
    );
  }
  if (!isset($selection['mail']) || ($selection['mail'] !== FALSE)) {
    $return['mail'] = array(
      '#type'      => 'textfield',
      '#title'     => t('email adress'),
      '#size'      => 60,
      '#maxlength' => EMAIL_MAX_LENGTH,
      '#required'  => isset($selection['mail']['required']) ? $selection['mail']['required'] : TRUE,
    );  
  }
  if (!isset($selection['telephone']) || ($selection['telephone'] !== FALSE)) {
    $return['telephone'] = array(
      '#type'      => 'textfield',
      '#title'     => t('phone'),
      '#size'      => 25,
      '#maxlength' => 25,
      '#required'  => isset($selection['telephone']['required']) ? $selection['telephone']['required'] : TRUE,
    );
  }
  if (!isset($selection['adresse_1']) || ($selection['adresse_1'] !== FALSE)) {
    $return['adresse_1'] = array(
      '#type'      => 'textfield',
      '#title'     => t('Adress (line 1)'),
      '#size'      => 60,
      '#maxlength' => 255,
      '#required'  => isset($selection['adresse_1']['required']) ? $selection['adresse_1']['required'] : TRUE,
    );
  }
  if (!isset($selection['adresse_2']) || ($selection['adresse_2'] !== FALSE)) {
    $return['adresse_2'] = array(
      '#type'      => 'textfield',
      '#title'     => t('Adress (line 2)'),
      '#size'      => 60,
      '#maxlength' => 255,
       '#required'  => isset($selection['adresse_2']['required']) ? $selection['adresse_2']['required'] : TRUE,
   );
  }
  if (!isset($selection['date_naissance']) ||  ($selection['date_naissance'] !== FALSE)) {
    $return['date_naissance'] = array(
      '#type'        => 'textfield',
      '#title'       => t('Birthdate'),
      '#description' => t('Birthdate format "jj-mm-aaaa"'),
      '#required'  => isset($selection['date_naissance']['required']) ? $selection['date_naissance']['required'] : TRUE,
   );
  }
  return $return;
 }
 
/**
 * Controle la date de naissance du stagiaire.
 *
 *  Vérifie si la date est valide et si l'âge du stagiaire est acceptable
 *  (entre 10 et 20 ans).)
 *  
 *  @param string $birthdate Date de naissance introduit par l'utilisateur
 *
 *  @return Le timestamp correspondant à la date introduite (si elle passe
 *    les tests).@n
 *    Dans le cas contraire, un message d'erreur.
 */
function _inscription_jesa_chek_birthdate($birthdate) {
  //  on remplace les / éventuels par des - ce qui force la fonction
  //  strtotime à analyser la date sous la forme jour/mois/annéé et non
  // mois/jour/année
  $date_naissance_dt = date_create(str_replace('/', '-', $birthdate));
  if ($date_naissance_dt == FALSE) {
    //la date n'est pas reconnue
    $return = t('The birthdate is not correct.');
  }
  else {
    //Contrôle si le stagiaire à un age 'logique' à savoir entre 10 et 20 ans
    $age = date_diff($date_naissance_dt, date_create());
    if ($age->y < 11 || $age->y > 20) {
      $return = t('The members\'age (!age) doesn\'t allow him to register to the event.', 
        array('!age'=> $age->y));
    }
    else {
      $return = date_timestamp_get($date_naissance_dt);
    }
  }
  return $return;
}


/**
 * Renvoie une liste d'utilisateurs pour un rôle.
 *
 *  Renvoie la liste d'utilisateurs dont le nom ou le prénom continnent 
 *  des caractères recherché et qui ont un rôle donné.
 *  
 *  @param string $search Caratères se trouvant dans le nom ou le prénom
 *  @param string $role   Rôle recherché (en toute lettre)
 *
 *  @return Un tableau contenant les informations de l'utilisateur@n
 *           - uid    identifiant de l'utilisateur
 *           - name   nom de connexion de l'utilisateur
 *           - nom    nom de l'utilisateur
 *           - prenom prénom de l'utilisateur
 *    Dans le cas contraire, un message d'erreur.
 */
function _inscription_jesa_get_users_with_role($search, $role) {
  $query = '
    SELECT 
       u.uid                AS uid,
       u.name               AS name,
      fn.field_nom_value    AS nom, 
      fp.field_prenom_value AS prenom
    FROM
      users u
      INNER JOIN users_roles ur ON
        u.uid = ur.uid
      INNER JOIN {role} r ON
        ur.rid = r.rid
      INNER JOIN field_data_field_nom fn ON
            u.uid          = fn.entity_id
        AND fn.entity_type = :entity_type
        AND fn.bundle      = :user_bundle
        AND fn.language    = :language
        AND fn.deleted     = 0
        AND fn.delta       = 0
      INNER JOIN field_data_field_prenom fp ON
            u.uid          = fp.entity_id
        AND fp.entity_type = :entity_type
        AND fp.bundle      = :user_bundle
        AND fp.language    = :language
        AND fp.deleted     = 0
        AND fp.delta       = 0
    WHERE  
          r.name   = :role
      AND u.status = 1
      AND (
               field_nom_value    LIKE :search_string 
            OR field_prenom_value LIKE :search_string
           )
   ';

try {
    $results = db_query_range(
      $query,
      0,
      10,
      array(  
        ':entity_type'   => 'user',
        ':user_bundle'   => 'user',
        ':role'          => $role,
        ':language'      => LANGUAGE_NONE,  //language indépendant
        ':search_string' => '%' . $search . '%',
      ),
      array('throw_exception' => TRUE,));
  }
  catch(PDOException $e) {
    watchdog_exception('jesa inscription', $e, 'DB error of %type: !message in %function (line %line of %file).', array(), WATCHDOG_ALERT, NULL);
    throw new ErrorException(
      t('Error during database access.'),
      0,
      E_USER_NOTICE
    );
  }
  return $results->fetchAll();
}