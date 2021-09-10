<?php
/**
 * @file 
 * Provide form api to backup page.
 * Fields:
 * - Select project.
 * - File upload.
 * - Textarea.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\file\Entity\File;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Render\Markup;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines RawphenotypesInstructionSearchForm class.
 */
class RawphenotypesBackupUploadForm extends FormBase {
  // Currently logged user - Drupal user id number.
  // This form will rely on this user to load only relevant
  // projects the user is appointed to.
  private $current_user;
  // User service.
  private $user_service;
  // Default Service.
  private $default_service;
  // Project Service.
  private $project_service;
  // Term service.
  private $term_service;


  /**
   * Initialized current user id and services.
   */
  public function __construct() {
    $this->term_service    = \Drupal::service('rawphenotypes.term_service');
    $this->default_service = \Drupal::service('rawphenotypes.default_service');
    $this->project_service = \Drupal::service('rawphenotypes.project_service');

    $this->user_service = \Drupal::service('rawphenotypes.user_service');
    $this->current_user = \Drupal::currentUser()
      ->id();
  }
  
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'rawphenotypes_search_trait_form';
  }

  /**
   * {@inheritdoc}
   * Build form.
   */
  public function buildForm(array $form, FormStateInterface $form_state, $project_id = NULL) { 
    // Create a select box containing projects available - these are projects
    // that have associated column header set and must have at least 1 essential column header.
    // The projects are filtered to show only projects assigned to user.
    $all_project = $this->user_service::getUserProjects($this->current_user);

    // No projects assined to the user.
    if (count($all_project) < 1) {
      return $form;
    }
    
    // FORM.
    // Construct backup file form.
    // Array to hold project assigned to user.
    $arr_my_project = $all_project;

    if (!isset($project_id) AND $project_id <= 0) {
      if (count($arr_my_project) > 0) {
        // Default to the only project when there is a single project assigned to user
        // otherwise, let user select a project.
        if (count($arr_my_project) > 1) {
          // When there is more that 1 project, tell user to select a project.
          $project_options = [0 => '---'] + $arr_my_project;
        }
        else {
          // Else, default to the only project available.
          $project_options = $arr_my_project;
        }
  
        
        // Yes, user has at least a project in the account.
        $form['backup_sel_project'] = [
          '#type' => 'select',
          '#title' => $this->t('Please select a project:'),
          '#options' => $project_options,
          '#default_value' => array_keys($project_options)[0],
          '#id' => 'backup_sel_project'
        ];

        $form['backup_txt_description'] = [
          '#type' => 'textarea',
          '#title' => $this->t('Describe spreadsheet file:'),
          '#description' => t('Use this field to add comments, description or notes to the spreadsheet file that you want to backup'),
          '#rows' => 2,
          '#resizable' => FALSE,
        ]; 

        $form['file_backup_file'] = [
          '#type' => 'file',
          '#title' => $this->t('<strong class="dragdrop-text">Drag your Microsoft Excel Spreadsheet file here or</strong> 
                               <span>Choose file</span>'),
          '#prefix' => '<div class="drop-zone"><div id="drop-zone-file">&nbsp;</div>',
          '#suffix' => '</div>',
          '#id' => 'field-file'
        ];

        $form['submit'] = [
          '#type' => 'submit',
          '#value' => 'Backup',
          '#id' => 'backup-submit-form'
        ];



      }
      else {
        // User has no project in the account.
        $form['message_invalid_project'] = [
          '#type' => 'inline_template',
          '#theme' => 'theme-rawphenotypes-message',
          '#data' => [
            'message' => $this->t('You have no project in your account. Please contact the administrator of this website.'),
            'type' => 'warning'
          ]
        ];
      }
    }
    else {
      // User is managing files in a project.
      // Upload is disabled. Display a summary,
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {    
    $get_field = $this->getRequest()->files->get('files', []);
    $file_field = $get_field['file_backup_file'];
    var_dump($get_field);
    die();
  }
}