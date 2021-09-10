<?php
/**
 * @file 
 * Page backup: page to enable user to backup data collection
 * spreadsheet file(s).
 */

namespace Drupal\rawphenotypes\Controller;

use Drupal\Core\Url;
use Drupal\Core\Controller\ControllerBase;

/**
 * Defines RawphenotypesPageBackupController class.
 * 
 * @see rawphenotypes/src/Form/RawphenotypesUploadForm
 * @see template-rawphenotypes-page-backup.html.twig
 */
class RawphenotypesPageBackupController extends ControllerBase {
  /**
   * Construct backup page.
   */
  public function loadPage($project_id = NULL) {
    // Build form elements - upload form.
    // see @see information - form and twig template.
    $form = \Drupal::formBuilder()
      ->getForm('Drupal\rawphenotypes\Form\RawphenotypesBackupUploadForm', $project_id);
    /*
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
    */  
    return [
      '#theme' => 'rawphenotypes_page_backup_template',
      '#upload_form' => $form,
      '#attached' => [
        'library' => 'rawphenotypes/page-backup',
        //'drupalSettings' => $to_Drupalsettings,
      ],
    ];
  }
}