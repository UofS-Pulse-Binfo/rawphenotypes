<?php
/**
 * @file 
 * Provide form api to instructions page.
 * Fields:
 * - Autocomplete search trait.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Render\Markup;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines RawphenotypesInstructionSearchForm class.
 */
class RawphenotypesInstructionsSearchForm extends FormBase {
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

    // Ensure that project is valid.
    if ($project_id) {
      if (!in_array($project_id, array_keys($all_project))) {
        // Project does not exist.
        $form['message_invalid_project'] = [
          '#type' => 'inline_template',
          '#theme' => 'theme-rawphenotypes-message',
          '#data' => [
            'message' => $this->t('Project does not exist.'),
            'type' => 'warning'
          ]
        ];
      }
      else {
        // Project is valid. Null this.
        $form['message_invalid_project'] = [];
      }
    }
    else {
      // When no project is supplied, then default this page to the most recent project
      // available from the projects defined by admin.
      $project_id = array_keys($all_project)[0];
    }

    // Make project id available to template.
    $form['project_id'] = $project_id;

    $form['txt_search'] = [
      '#title' => '',
      '#type' => 'textfield',
      '#maxlength' => 65,
      '#size' => 65,
      '#default_value' => $this->t('Search Trait'),
      '#autocomplete_route_name' => 'rawphenotypes.autocomplete.term',
      '#autocomplete_route_parameters' => ['project_id' => $project_id],
      '#theme_wrappers' => [],
    ];
    
    // Given a project id, construct the table required for each trait type set.
    // All trait types, need to remove type plantproperty.
    $trait_set = $this->default_service::getTraitTypes();
    $project_terms = $this->project_service::getProjectTerms($project_id);

     // Array to hold trait row.
    $arr_cvterm = [];

    foreach($project_terms as $term) {
      if ($term->type != $trait_set['type4']) {
        // Exclude plantproperty types from official list
        // of traits of the project.
        $cvterm = $this->term_service->getTermProperties($term->project_cvterm_id);
        $arr_cvterm[ $cvterm['type'] ][] = [
          Markup::create('<div class="data-cells">' . $cvterm['name'] . '</div>'),
          $cvterm['method'],
          $cvterm['definition']
        ];
      }
    }

    $arr_tbl_args['empty'] = $this->t('0 Column Header');
    $arr_tbl_args['header'] = [$this->t('Column Header/Trait'), $this->t('Collection Method'), $this->t('Definition')];
    
    unset($trait_set['type4']);

    foreach($trait_set as $type) {
      $arr_tbl_args['rows'] = isset($arr_cvterm[ $type ]) ? $arr_cvterm[ $type ] : [];

      if (count($arr_tbl_args['rows']) > 0) {
        $form['tbl_project_headers_' . $type] = [
          '#type' => 'table',
          '#title' => $this->t($type),
          '#header' => $arr_tbl_args['header'],
          '#rows' => $arr_tbl_args['rows']
        ];
      }
      else {
        $form['tbl_project_headers_' . $type] = FALSE;
      }
    }
    
    $form['active_project'] = [
      '#type' => 'inline_template',
      '#template' => $all_project[ $project_id ],

      '#empty' => $arr_tbl_args['empty']
    ];

    $form['sel_project'] = array(
      '#type' => 'select',
      '#options' => array(0 => 'Please select a project') + $all_project,
      '#id' => 'rawpheno-ins-sel-project'
    );

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
  }
}