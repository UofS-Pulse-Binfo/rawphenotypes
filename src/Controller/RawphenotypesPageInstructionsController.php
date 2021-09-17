<?php
/**
 * @file 
 * Page instructions: contains standard phenotyping instructions,
 * traits listings and photo reference.
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\Core\Url;
use Drupal\Core\Controller\ControllerBase;

/**
 * Defines RawphenotypesPageInstructionsController class.
 * 
 * @see rawphenotypes/src/Form/RawphenotypesInstructionsSearchForm
 * @see template-rawphenotypes-page-instructions.html.twig
 */
class RawphenotypesPageInstructionsController extends ControllerBase {
  /**
   * Construct instructions page.
   */
  public function loadPage($project_id = NULL) {
    // Build form elements - search trait in instructions page.
    // see @see information - form and twig template.
    $form = \Drupal::formBuilder()
      ->getForm('Drupal\rawphenotypes\Form\RawphenotypesInstructionsSearchForm', $project_id);
    
    // Pass route to instructions page with the selected project in the
    // select project (change project) as route parameter.
    $instructions_route = Url::fromRoute('rawphenotypes.page_instructions', [], ['absolute' => TRUE])
      ->toString();
    $to_Drupalsettings['rawphenotypes']['vars']['instructions_route'] = $instructions_route;

    // Image gallery path to photo appendix.
    $gallery_path = \Drupal::service('extension.list.module')
      ->getPath('rawphenotypes');
    global $base_url;  
    $to_Drupalsettings['rawphenotypes']['vars']['image_gallery_path'] = $base_url. base_path() . $gallery_path . '/images/appendix/';
      
    return [
      '#theme' => 'rawphenotypes_page_instructions_template',
      '#search_form' => $form,
      '#attached' => [
        'library' => 'rawphenotypes/page-instructions',
        'drupalSettings' => $to_Drupalsettings,
      ],
    ];
  }
}