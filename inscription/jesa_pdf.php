<?php
// Extend the TCPDF class to create custom Header and Footer
require_once(DRUPAL_ROOT . '/sites/all/libraries/tcpdf/tcpdf.php');
 
/**
 * Extension de la classe TCPDF pour créer des entête et pied de page sur mesure
 * 
 * 
 */
class JESAPDF extends TCPDF {
	/**
  *  Constructeur
  * 
  * Définit également les valeurs par défaut. 
  */
	function __construct() {
		parent::__construct();
		// set document information
		$this->SetCreator(PDF_CREATOR);
		$this->SetAuthor('Jean-Paul Scandariato');
		$this->SetKeywords('Jeunesse et Science, astronomie, stage');
    $this->SetSubject('Stage d\'astronomie');
		
		// set header and footer fonts
		$this->setHeaderFont(Array('helvetica', 'B', 9));
		$this->setFooterFont(Array(PDF_FONT_NAME_DATA, '', PDF_FONT_SIZE_DATA));
		$this->SetHeaderMargin(PDF_MARGIN_HEADER);
		$this->SetFooterMargin(PDF_MARGIN_FOOTER);
		
		// set default monospaced font
		$this->SetDefaultMonospacedFont(PDF_FONT_MONOSPACED);
		
		//set margins
		$this->SetMargins(PDF_MARGIN_LEFT, PDF_MARGIN_TOP, PDF_MARGIN_RIGHT);
	
  	// set color for background
    $this->SetFillColor(255, 255, 255);
		
		//set auto page breaks
		$this->SetAutoPageBreak(TRUE, PDF_MARGIN_BOTTOM);
		
		//set image scale factor
		$this->setImageScale(PDF_IMAGE_SCALE_RATIO);
		
		//set some language-dependent strings
		$this->setLanguageArray(
		array(
		'a_meta_charset'  => 'UTF-8',
		'a_meta_dir'      => 'ltr',
		'a_meta_language' => 'fr',
		'w_page'          => 'page',
		)
			);
	}
		
	/**
  * 
  * Générateur de l'entête personalisée.
  */
	public function Header() {
    $headerdata = $this->getHeaderData();
		// Logo		
		$this->Image($headerdata['logo'], 10, 10, $headerdata['logo_width'], '', 'JPG', 'http://www.jeunesse-et-science.be', 'T', false, 300, '', false, false, 1, false, false, false);

		// Set font
		$headerfont = $this->getHeaderFont();
		$this->SetFont($headerfont[0], 'B', $headerfont[2] + 1);
		// Title
		$this->Cell(0, 16.1, (isset($headerdata['title']) ? $headerdata['title'] : '') , 1, false, 'C', 0, '', 0, false, 'T', 'M');
	}
	
	/**
  * 
  * Générateur du pied de page personalisé.
  */
	public function Footer() {
		// Position at 15 mm from bottom
		$this->SetY(-20);
		// Set font
		$this->SetFont('helvetica', 'I', 8);
    $txt = 'Jeunesse et Sciences asbl';
		$this->Cell(70, 0, $txt, 'T', false, 'L', 0, '', 0, false, 'T', 'M');

		$this->Cell(30, 0, 'Page '.$this->getAliasNumPage().'/'.$this->getAliasNbPages(), 'T', false, 'C', 0, '', 0, false, 'T', 'M');

    $txt = 'numéro d\'entreprise 0415.456.146';
		$this->Cell(70, 0, $txt, 'T', 1, 'R', 0, '', 0, false, 'T', 'M');

    $txt = 'rue des Hippocampes 9';
		$this->Cell(70, 0, $txt, 0, 1, 'L', 0, '', 0, false, 'T', 'M');

   $txt = '1080 Bruxelles';
 		$this->Cell(70, 0, $txt, 0, 1, 'L', 0, '', 0, false, 'T', 'M');
	}
}