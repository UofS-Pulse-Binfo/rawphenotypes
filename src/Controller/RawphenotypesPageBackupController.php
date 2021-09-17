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
    
    // Drag and drop callback.
    $to_Drupalsettings['rawphenotypes']['vars']['dragdropfile'] = Url::fromRoute('rawphenotypes.callback.dragdropfile', [], ['absolute' => TRUE])
      ->toString();

    return [
      '#theme' => 'rawphenotypes_page_backup_template',
      '#upload_form' => $form,
      '#attached' => [
        'library' => 'rawphenotypes/page-backup',
        'drupalSettings' => $to_Drupalsettings,
      ],
    ];
  }
}