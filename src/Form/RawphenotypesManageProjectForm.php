<?php
/**
 * @file 
 * Construct configuration form to manage phenotyping experiments.
 */

namespace Drupal\rawphenotypes\Form;

use Drupal\Core\Url;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Defines RawphenotypesRTransformForm class.
 */
class RawphenotypesManageProjectForm extends FormBase {
  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'rawphenotypes_projects';
  }

  /**
   * {@inheritdoc}
   * Build form.
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    // Attach library.
    $form['#attached']['library'][] = 'rawphenotypes/style-admin';
    
    // Load services.
    $project_service = \Drupal::service('rawphenotypes.project_service');
    $default_service = \Drupal::service('rawphenotypes.default_service');

    $new_projects = $project_service::getNewProjects();

    $form['fieldset_sel_project'] = [
      '#type' => 'details',
      '#title' => $this->t('Create a New Project:'),
      '#open' => TRUE,
    ];

    if ($new_projects) {
      $options = array('0' => '---') + $new_projects;

      $form['fieldset_sel_project']['sel_project'] = [
        '#type' => 'select',
        '#title' => $this->t('Please select a project:'),
        '#options' => $options,
        '#default_value' => array_keys($options)[0],
        '#id' => 'admin-sel-project',
        '#description' => $this->t('Please note that column headers Plot, Entry, Rep, Name, Location and Planting Date (date) are added to project by default.'),
        '#theme_wrappers' => []
      ];
  
      $form['fieldset_sel_project']['add_project'] = [
        '#type' => 'submit',
        '#value' => $this->t('Create Project'),
        '#id' => 'admin-add-project',
      ];
    }
    else {
      $form['fieldset_sel_project']['no_project'] = [
        '#type' => 'inline_template',
        '#theme' => 'theme-rawphenotypes-message',
        '#data' => [
          'message' => $this->t('No projects available.'),
          'type' => 'warning'
        ]
      ];
    }

    // TABLE
    // Construct table that lists all active projects in this module.
    // The table data consists of project name and summary of column headers and active users.

    // Get trait types array.
    $trait_type = $default_service::getTraitTypes();

    // Array to hold table headers.
    $arr_headers = array();

    // Array to hold table rows.
    $arr_rows = array();

    $empty_table_title = "";
    
    $projects = $project_service::getActiveProjects();

    // Page title.
    $form['all_project_info'] = array(
      '#type' => 'inline_template',
      '#template' => '<h2>' . count($projects) . ' Project(s)</h2>'
    );

    if (count($projects) > 0) {
      $i = 0;

      foreach($projects as $p) {
        $link = Url::fromRoute('rawphenotypes.manage_project', ['asset_id' => $p->project_id, 'asset_type' => 'project', 'action' => 'manage']);
        $view_cell = \Drupal::l($this->t('View'), $link);
        $name_cell = \Drupal::l($this->t($p->name), $link);
  
        // Warn user that the project has no essential trait. Project must have at least 1 essential trait
        // list of column headers before it can process and store phenotypic data. The same for user.
        $warn_header = ($p->essential_count > 0)
          ? ''
          : 'Project has no essential column header';
  
        $warn_user = ($p->user_count > 0)
          ? ''
          : 'Project has no assigned user';
  
  
        // + 1 to account for Name column header.
        array_push($arr_rows, array(($i+1), $name_cell, ($p->header_count + 1), $p->essential_count . $warn_header, $p->user_count . $warn_user, $view_cell));
        $i++;
      }
    }
    else {
      $empty_table_title = $this->t('No project available');
    }

    array_push($arr_headers, '-', $this->t('Project Name'), $this->t('Column Headers'), $this->t('Essential Headers'), $this->t('Active User'), $this->t('View'));

    $form['tbl_projects'] = [
      '#type' => 'table',
      '#title' => $this->t('Projects'),
      '#header' => $arr_headers,
      '#rows' => $arr_rows,
      '#empty' => $empty_table_title,
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    $project_id = $form_state->getValue('sel_project');
    
    if ($project_id <= 0) {
      $form_state->setErrorByName('sel_project', $this->t('No project selected. Please select a project and try again'));
    }
  }

  /**
   * {@inheritdoc}
   * Save configuration.
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    // Load services.
    $term_service    = \Drupal::service('rawphenotypes.term_service');
    $project_service = \Drupal::service('rawphenotypes.project_service');
    $default_service = \Drupal::service('rawphenotypes.default_service');

    // When a new project is created, Plant property column headers are
    // added to a project by default. The following are the plant property headers:
    // Location, Plot Rep and Entry.

    // Plus and Planting Date and make it essential trait. Planting date is important
    // as it is used when generating the heatmap.
    $project_id = $form_state->getValue('sel_project');

    // Get trait types array.
    $trait_type = $default_service::getTraitTypes();

    // Get Planting Date cvterm id.
    $pd = ['name' => 'Planting Date (date)', 'cv_id' => ['name' => 'phenotype_measurement_types']];
    $planting_date = $term_service::getTerm($pd);
    $term_service::saveTermToProject($planting_date->cvterm_id, $trait_type['type1'], $project_id);
    
    // Query all plant property types column headers in chado.cvterm table.
    // Insert to project.
    $plantprop = $term_service::getPlantPropertyTerm();
    foreach($plantprop as $prop) {
      $term_service::saveTermToProject($prop->cvterm_id, $trait_type['type4'], $project_id);
    }    

    $project_service::addUserToProject(\Drupal::currentUser()->id(), $project_id);
    // If the user is not the superadmin, add superadmin as well.
    if (\Drupal::currentUser()->id() != 1) {
      $project_service::addUserToProject(1, $project_id);
    }    
  }
}