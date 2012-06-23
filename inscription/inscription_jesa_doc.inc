<?php
/**
*  Fonction de génération des documents d'inscirptions.
* 
* Contient deux documents et leurs fonctions associées.
* - L'inscription d'un stagiaire.
* - L'autorisation photographique.
*  @file
*  @author    Jean-Paul Scandariato
*  @date      10/06/2012
*/

  require_once(dirname(__FILE__) . '/jesa_pdf.php');

/**
* Formate la date pour être affichée
*
* 
* @param string $date Date au format yyyy-mm-ddThh:mm:ss
* 
* @return la date au format dd/mm/yyyy 
*/ 
function _jesa_inscription_format_date($date) {
  if (strlen($date >= 10)) {
    return substr($date,8,2) . '/' . substr($date,5,2) . '/' . substr($date,0,4);   
  }
  else {
    return '';
  }
}

/**
* Crée le fichier d'inscription au stage rempli avec les informations du stagiaire.
* 
* @param node $node_stagiaire node de l'inscription au stage
* 
*/
function _jesa_inscription_generate_inscription_file($node_stagiaire) {

  $annee = $node_stagiaire->field_stagiaire_annee[LANGUAGE_NONE][0]['value'];
  $train = $node_stagiaire->field_stagiaire_train[LANGUAGE_NONE][0]['value'];
  $stagiaire = user_load($node_stagiaire->field_stagiaire_user[LANGUAGE_NONE][0]['uid']);
  
  $stage = node_load($node_stagiaire->field_stagiaire_stage[LANGUAGE_NONE][0]['nid']);
  
  
 // require_once(dirname(__FILE__) . '/jesa_pdf.php');
  
  // create new PDF document
  $pdf = new JESAPDF(PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);
  
  // set default header data
  $txt = 'Bulletin d\'inscription à un stage d\'astronomie';
  $pdf->SetHeaderData(dirname(__FILE__) . '/img/logo_jesa.jpg', 26, $txt, '');
  $pdf->SetTitle('Formulaire d\'inscription');
  
  // add a page
  $pdf->AddPage();
  
  
  $pdf->SetFont('helvetica', '', 12);
  $pdf->Ln(10);
  
  
  $txt = 'Nom : ' . $stagiaire->field_nom[LANGUAGE_NONE][0]['value'] . "\n";
  $txt .= "\n";
  if (isset($stagiaire->field_date_naissance[LANGUAGE_NONE][0]['value'])) {
    $naissance = $stagiaire->field_date_naissance[LANGUAGE_NONE][0]['value'];
    $txt .= 'Date de naissance :  ' . _jesa_inscription_format_date($naissance);
  }
  else {
    $txt .= 'Date de naissance :  ........../........../...............';
  }
  
  
  $pdf->MultiCell(95, 20, $txt, 0, 'L', 1, 0, '', '', true);
  
  $txt = 'Prénom : ' . $stagiaire->field_prenom[LANGUAGE_NONE][0]['value'] . "\n";
  $txt .= "\n";
  if (isset($stagiaire->field_sexe[LANGUAGE_NONE][0]['value'])) {
   $txt .= 'Sexe :   ' . ($stagiaire->field_sexe[LANGUAGE_NONE][0]['value'] == 'M' ? 'X' : ' ') . '  M     ' . ($stagiaire->field_sexe[LANGUAGE_NONE][0]['value'] == 'F' ? 'X' : ' ') . ' F'; 
  }
  else {
    $txt .= 'Sexe :   o M     o F';  
  }
  
  $pdf->MultiCell(95, 20, $txt, 0, 'L', 1, 1, '', '', true);
  
  if (isset($stagiaire->field_adresse_1[LANGUAGE_NONE][0]['value'])) {
    $txt = 'Adresse : ' . $stagiaire->field_adresse_1[LANGUAGE_NONE][0]['value'] . "\n";
    $txt .= "\n";
    $txt .= '               ' . $stagiaire->field_adresse_2[LANGUAGE_NONE][0]['value'] . "\n";
  } 
  else {
    $txt = 'Adresse : ..........................................................................................' . "\n";
    $txt .= "\n";
    $txt .= 'Code postal : .............. Localité :.......................................................';
  }
  
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='L', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  $pdf->Ln(4);
  
  
  if (isset($stagiaire->field_tel[LANGUAGE_NONE][0]['value'])) {
    $txt = 'Téléphone : ' . $stagiaire->field_tel[LANGUAGE_NONE][0]['value'] . "\n";
  }
  else {
    $txt = 'Téléphone : .............................' . "\n";
  }
  $txt .= "\n";
  $txt .= 'école : ....................................................';
  $pdf->MultiCell(95, 5, $txt, 0, 'L', 1, 0, '', '', true);
  
  $txt = 'e-mail : ' . $stagiaire->mail . "\n";
  $txt .= "\n";
  $txt .= 'Classe : ' . $annee;
  $pdf->MultiCell(95, 5, $txt, 0, 'L', 1, 1, '', '', true);
  $pdf->Ln(4);
  
  $txt = 'Stage d\'astronomie  se déroulant à Modave du ' . _jesa_inscription_format_date($stage->field_stage_dates[LANGUAGE_NONE][0]['value']) . ' au ' . _jesa_inscription_format_date($stage->field_stage_dates[LANGUAGE_NONE][0]['value2']);
  $txt .= ' et dont le prix s\'élève à ' . $stage->field_stage_prix[LANGUAGE_NONE][0]['value'] . ' €, à payer sur place.' . "\n";
  $txt .= "\n";
  $txt .= 'Viendras-tu par les transports en commun ? ' . ($train ? 'oui' : 'non') . "\n";
  $txt .= "\n";
  
  $txt .= <<<EOD
  Date : ........../........../...............  
  
  Signature du stagiaire : .......................................
  
  
  À remplir si le (la) candidat(e) est mineur(e)
  
  Le soussigné, exerçant la puissance parentale sur le (la) mineur(e) d'âge désigné ci dessus,
  
    - l'autorise à participer au stage indiqué ;
    - déclare savoir que la responsabilité du cadre ne s'exerce que pendant le stage dont la durée est définie dans ce document, à l'exclusion notamment des trajets aller et retour ;
    - affirme de bonne foi que le (la) candidat(e) est dans un état de santé tel qu'il (elle) ne peut compromettre celle des autres participants.         
  
  (écrire à la main " Lu et approuvé ")
  
  
  Date : ........../........../...............  
  
  Signature : ......................................................
  
  Nom du signataire (o père o mère o tuteur) : ...........................
EOD;
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='L', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  
  _jesa_inscription_save_document($pdf, 'Inscription.' . $node_stagiaire->nid);
}

/**
* Crée le fichier de demande d'autorisation photographique rempli avec les informations du stagiaire.
* 
* @param node $node_stagiaire node de l'inscription au stage
* @param uid  $parent_uid identification du parent ayant fait l'inscription
* 
*/
function _jesa_inscription_generate_autorisation_file($node_stagiaire, $parent_uid) {
  
  $stagiaire = user_load($node_stagiaire->field_stagiaire_user[LANGUAGE_NONE][0]['uid']);
  $stage = node_load($node_stagiaire->field_stagiaire_stage[LANGUAGE_NONE][0]['nid']);
  $parent = user_load($parent_uid);
  

  
  // create new PDF document
  $pdf = new JESAPDF(PDF_PAGE_ORIENTATION, PDF_UNIT, PDF_PAGE_FORMAT, true, 'UTF-8', false);
  
  // set default header data
  $txt = 'AUTORISATION DE REPRODUCTION ET DE REPRESENTATION DE PHOTOGRAPHIES';
  $pdf->SetHeaderData(dirname(__FILE__) . '/img/logo_jesa.jpg', 26, $txt, '');
  $pdf->SetTitle('Formulaire d\'autorisation photographique');
  
  // add a page
  $pdf->AddPage();
  $pdf->SetFont('helvetica', '', 8);
  
  
  $prenom_nom_stagiaire = $stagiaire->field_prenom[LANGUAGE_NONE][0]['value']. ' ' . $stagiaire->field_nom[LANGUAGE_NONE][0]['value'];
  $prenom_nom_parent = $parent->field_prenom[LANGUAGE_NONE][0]['value']. ' ' . $parent->field_nom[LANGUAGE_NONE][0]['value'];
  $stage_date_debut = _jesa_inscription_format_date($stage->field_stage_dates[LANGUAGE_NONE][0]['value']);
  $stage_date_fin = _jesa_inscription_format_date($stage->field_stage_dates[LANGUAGE_NONE][0]['value2']);
  $today = strftime('%#d/%m/%Y',time());
  
  $pdf->Ln(20);
  
  $txt = 'Bruxelles, le ' . $today . '.' . "\n";;
  $txt .= "\n";
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='R', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  
  $pdf->Ln(10);
  $txt = <<<EOD
  Je soussigné(e) $prenom_nom_parent, parent ou responsable de $prenom_nom_stagiaire, accepte par la présente que les photos sur lesquelles figure $prenom_nom_stagiaire, prises lors de sa participation au stage d'astronomie du $stage_date_debut au $stage_date_fin de l’asbl Jeunesse et Science, puissent être utilisées uniquement à des fins professionnelles par l’asbl Jeunesse et Science. 
  
  Les photos pourront être publiées sous toute forme et tous supports connus et inconnus à ce jour, dans le monde entier, sans limitation de durée, intégralement ou par extraits et notamment : 
  - Presse, 
  - Livre, 
  - Exposition, 
  - Publicité, 
  - Projection publique, 
  ainsi que sur le site internet de l’asbl http://www.jeunesse-et-science.be et http://www.jesa.be. 
EOD;
  $txt .= "\n";
  $txt .= "\n";
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='J', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  
  $txt = <<<EOD
  L’asbl Jeunesse et Science s’interdit expressément de procéder à une exploitation des photographies susceptible de porter atteinte à la vie privée ou à la réputation, ni d’utiliser les photographies de la présente, dans tout support à caractère pornographique, raciste, xénophobe ou toute autre exploitation préjudiciable. 
EOD;
  $txt .= "\n";
  $txt .= "\n";
  $pdf->SetFont('helvetica', 'B', 8);
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='J', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  $pdf->SetFont('helvetica', '', 8);
  
  $txt = <<<EOD
  L’asbl Jeunesse et Science s’efforcera dans la mesure du possible, de tenir à disposition un justificatif de chaque parution des photographies sur simple demande. 
  En aucun cas, l’association ne cédera les photos visées à des tiers.
EOD;
  $txt .= "\n";
  $txt .= "\n";
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='J', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  
  $txt = <<<EOD
  Nous ne pourrons prétendre à aucune rémunération pour l’exploitation des droits visés aux présentes. 
  Je garantis que je ne suis pas lié par un contrat exclusif relatif à l’utilisation de mon image ou de mon nom. 
EOD;
  $txt .= "\n";
  $txt .= "\n";
  $pdf->SetFont('helvetica', 'B', 8);
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='J', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  $pdf->SetFont('helvetica', '', 8);
  
  $pdf->Ln(10);
  
  $txt = <<<EOD
  Signature précédée de la mention manuscrite "Lu et approuvé" : 
EOD;
  $pdf->SetFont('helvetica', 'I', 8);
  $pdf->Write($h=0, $txt, $link='', $fill=0, $align='L', $ln=true, $stretch=0, $firstline=false, $firstblock=false, $maxh=0);
  $pdf->SetFont('helvetica', '', 8);
  
  
  $pdf->Ln(10);
  
  $txt = 'Responsable parental' . "\n";
  $txt .= $prenom_nom_parent . "\n";
  $txt .= "\n";
  $txt .= "\n";
  $txt .= '………………………………………………' . "\n";
  $txt .= "\n";
  $txt .= "\n";
  $txt .= '………………………………………………';
  $pdf->MultiCell(95, 5, $txt, 0, 'L', 1, 0, '', '', true);
  
  $txt = 'Personne photographiée' . "\n";
  $txt .= $prenom_nom_stagiaire . "\n";
  $txt .= "\n";
  $txt .= "\n";
  $txt .= '………………………………………………' . "\n";
  $txt .= "\n";
  $txt .= "\n";
  $txt .= '………………………………………………';
  $pdf->MultiCell(95, 5, $txt, 0, 'L', 1, 1, '', '', true);
   
  _jesa_inscription_save_document($pdf, 'Autorisation.' . $node_stagiaire->nid);
}

/**
* Sauve un document PDF
*
* Sauve le document dans le répertoire temporaire (en supprimant la version précédente)
* 
* @param JESAPDF $pdf Object contenant le document à sauver
* @param string $doc_name nom du fichier
* 
*/
function _jesa_inscription_save_document($pdf, $doc_name) {
  $destination = 'temporary://jesa/' . $doc_name . '.pdf';
  //doit écrire la routine de suppression car unlink retourn une erreur E_WARNING qui s'affiche => non souhaitable.
  $can_write = FALSE;
  if (file_exists($destination)) {
    $can_write = @unlink($destination); 
  } else {
    $can_write = TRUE;  
  }
  
  if ($can_write) {
    //Close and output PDF document
    $pdf->Output($destination, 'F');    
  } else {
    //TODO si on ne peut pas supprimer le fichier on a un problème. Normalement cela ne devrait jamais se produire
    watchdog('jesa inscription', 'The file @file cannot be deleted', array('@file'=>$destination),WATCHDOG_ERROR);
  }
}