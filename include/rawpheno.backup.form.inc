<?php
/**
 * @file
 * Contains backup interface for this module.
 */
  
/**
 * Function callback: construct backup interface.
 */
function rawpheno_backup($form, &$form_state) {
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.backup.style.css');

  // Navigation button. Related page of rawdata/summary page is download page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Standard Procedure &#10095;'),
  );

  // Test if users have assigned project(s) in their account.
  // When there is no project, instruct user to create one and
  // only then they can start uploading files for backup.
  $user_id = $GLOBALS['user']->uid;
  
  // Query all project and flag if each project belongs to user account.
  // Read each result and sort the project to user project and projects that user can add.
  $sql = "
    SELECT DISTINCT t1.name, t1.project_id, CASE WHEN t2.uid = :uid THEN 'y' ELSE 'n' END AS flag 
    FROM chado.project AS t1 LEFT JOIN pheno_project_user AS t2 USING(project_id) 
      INNER JOIN pheno_project_cvterms AS t3 USING(project_id)
    WHERE t3.id IS NOT NULL";
  
  $args = array(':uid' => $user_id);
  $all_prj = db_query($sql, $args);
  
  $my_project = array();
  $not_my_prj = array();

  foreach($all_prj as $prj) {
    if ($prj->flag == 'y') {
      $my_project[$prj->project_id] = $prj->name;
    }
    else {
      $not_my_prj[$prj->project_id] = $prj->name; 
    }    
  }

  if (count($my_project) > 0) {
    $form['backup_sel_project'] = array(
       '#type' => 'select',
       '#title' => t('Please select a project:'),
       '#options' => $my_project,
       '#default_value' => 0,
       '#id' => 'backup_sel_project'
    );
 
    // Dropzone form element.
    $form['bdnd'] = array(
      '#type' => 'dragndrop_upload',
      '#title' => t('Spreadsheet file:'),
      '#file_upload_max_size' => '10M',
      '#upload_location' => 'public://',
      '#upload_event' => 'manual',
      '#upload_validators' => array('file_validate_extensions' => array('xlsx xls')),
      '#droppable_area_text' => t('Drag your Microsoft Excel Spreadsheet file here'),
      '#progress_indicator' => 'throbber',
      '#progress_message' => 'Please wait...',
      '#allow_replace' => 1,
      '#standard_upload' => 1,
      '#id' => 'backup_dropzone',
      '#upload_button_text' => t('Upload'),
    );

    $form['backup_txt_description'] = array(
      '#type' => 'textarea',
      '#title' => t('Describe spreadsheet file:'),
      '#description' => t('Use this field to add comments, description or notes to the spreadsheet file that you want to backup'),
      '#rows' => 2,
      '#resizable' => FALSE,
    );
  }
  else {
    // Inform user no project is assigned to the account.
    // Display user project file table to allow user to add project to account.
    $form['no_data'] = array(
      '#markup' => 'You have no project in your account. Please add project.',
      '#prefix' => '<div id="container-no-info" class="messages warning">',
      '#suffix' => '</div>',
    );
  }
  
  // ADD PROJECT FORM
  if (count($not_my_prj) > 0) {
    $form['add_project_select'] = array(
      '#type' => 'select',
      '#options' => array(0 => 'Select project to add to your account') + $not_my_prj,
      '#id' => 'add_project_select',
    );
  
    $form['add_project_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add Project'),
      '#id' => 'add_project_submit',
      '#limit_validation_errors' => array(
        array('add_project_select')
      ),
    );
  }
  else {
    $form['no_prj'] = array(
      '#type' => 'markup',
      '#markup' => '<em>No project available.</em>',
    );
  }

  // PROJECTS TABLE
  // Project file table.
  $sql = "
    SELECT COUNT(t2.file_id) AS file_count, t1.project_id 
    FROM pheno_project_user AS t1 LEFT JOIN pheno_backup_file AS t2 USING(project_user_id)
    WHERE t1.uid = :uid
    GROUP BY project_user_id, t1.project_id
    ORDER BY t1.project_id DESC";

  $my_file = db_query($sql, $args);
 
  $headers = array('Project', 'File', 'View');
  $rows = array();

  foreach($my_file as $file) {
    $rows[] = array($my_project[$file->project_id], $file->file_count, '<a href="">view</a>');
  } 
  
  $tbl_args = array('header' => $headers, 
                    'rows' => $rows, 
                    'sticky' => FALSE, 
                    'empty' => 'No project');
   
  $form['tbl_root_dir'] = array(
    '#markup' => theme('table', $tbl_args),
  );
  
  // Attach JavaScript.
  $form['#attached']['js']  = array($path . 'js/rawpheno.backup.script.js');

  return $form;
}

/**
 * Implements hook_submit().
 * Add project to user account.
 */
function rawpheno_backup_submit($form, &$form_state) {
  $user_id = $GLOBALS['user']->uid;
  $project_id = $form_state['values']['add_project_select'];
  
  db_insert('pheno_project_user')
    ->fields(array('project_id' => $project_id,
                   'uid' => $user_id))
    ->execute();    
}