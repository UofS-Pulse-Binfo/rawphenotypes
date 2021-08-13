<?php
/**
 * @file 
 * Page instructions: contains standard phenotyping instructions,
 * traits listings and photo reference.
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\Core\Controller\ControllerBase;

/**
 * Defines RawphenotypesPageInstructionsController class.
 */
class RawphenotypesPageInstructionsController extends ControllerBase {
  /**
   * 
   */


   
  public function loadPage() {

    $form = \Drupal::formBuilder()->getForm('Drupal\rawphenotypes\Form\RawphenotypesInstructionsSearchForm');
    
    return [
      '#theme' => 'rawphenotypes_page_instructions_template',
      '#search_form' => $form,
      '#attached' => [
        'library' => 'rawphenotypes/page-instructions'
      ]
    ];
  }
}