<?php
/**
 * @file
 * Contains administrative interfaces of this module.
 */

/**
 * Function callback: Construct main admin page of Rawphenotypes.
 */
function rawpheno_admin_main_page($form, &$form_state) {
  // Create a link directory to administrative pages.
  $links = '<br /><p>&bull; ' . l('Page Configurations', 'admin/tripal/extension/rawphenotypes/rawpheno_config')
    . '<br />&nbsp; Apply colour scheme and set page title.</p><hr />';

  $links .= '<p>&bull; ' . l('Define R Transformation Rules', 'admin/tripal/extension/rawphenotypes/rawpheno_rheaders')
    . '<br />&nbsp; Define R Transfomation Rules to be applied when generating R Friendly alternative of column header.</p><hr />';
  
  $links .= '<p>&bull; ' . l('Manage Projects', 'admin/tripal/extension/rawphenotypes/all_projects')
    . '<br />&nbsp; Create project, define column headers and appoint users to project.</p>';
  
  $form['rawphenotypes'] = array(
    '#markup' => '<h2>Rawphenotypes Module</h2>' . $links
  );
  
  return $form;
}

 /**
 * Function callback: Construct administrative interface to modify page title and colour scheme.
 */
function rawpheno_admin_page($form, &$form_state) {
  // Colour scheme.
  $form['fieldset_colour_scheme'] = array(
    '#type' => 'fieldset',
    '#title' => t('Colour Scheme'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );

    $form['fieldset_colour_scheme']['rawpheno_colour_scheme'] = array(
      '#type' => 'textfield',
      '#title' => t('Enter colour:'),
      '#default_value' => variable_get('rawpheno_colour_scheme'),
      '#size' => 40,
      '#maxlength' => 20,
      '#description' => t('eg. HEX: #304356, blue | <a href="@adobe-kuler" target="_blank">get more colours here</a>', 
                          array('@adobe-kuler' => url('https://color.adobe.com/'))),
      '#required' => TRUE,
    );
  
  // Headers and title.
  $form['fieldset_page_title'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page title'),
    '#collapsed' => FALSE,
    '#collapsible' => TRUE,
  );
    
    // Rawdata page.
    $form['fieldset_page_title']['rawpheno_rawdata_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Rawdata page:'),
      '#default_value' => variable_get('rawpheno_rawdata_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );
    
    // Download Page.
    $form['fieldset_page_title']['rawpheno_download_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Download page:'),
      '#default_value' => variable_get('rawpheno_download_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
    
    // Instructions page.
    $form['fieldset_page_title']['rawpheno_instructions_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Instructions page:'),
      '#default_value' => variable_get('rawpheno_instructions_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
  
    // Upload page.
    $form['fieldset_page_title']['rawpheno_upload_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Upload page:'),
      '#default_value' => variable_get('rawpheno_upload_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  

    // Backup page.
    $form['fieldset_page_title']['rawpheno_backup_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Backup page:'),
      '#default_value' => variable_get('rawpheno_backup_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );
  
  $form['req']['#suffix'] = t('<span>&nbsp;* means field is required</span>');  
  
  // This will add a save button to this form, thus 
  // no hook_submit nor hook_validate is required.
  return system_settings_form($form);
}


// Include function to manage column headers, cv terms and variable names.
module_load_include('inc', 'rawpheno', 'include/rawpheno.function.measurements');


/**
 * Function callback: Construct administrative interface to manage R Friendly transformation of column headers.
 */
function rawpheno_admin_rheaders($form, &$form_state) {
  // Fieldset to contain R-Friendly transformation rules form.
  $form['fieldset_rules'] = array(
    '#type' => 'fieldset',
    '#title' => t('R-Friendly Transformation Rules'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  ); 
  
    $form['fieldset_rules']['rawpheno_rtransform_words'] = array(
      '#type' => 'textarea',
      '#title' => t('List of words to remove:'),
      '#default_value' => variable_get('rawpheno_rtransform_words'),
      '#description' => 'Separate words with commas',
      '#required' => TRUE,
    );

    $form['fieldset_rules']['rawpheno_rtransform_characters'] = array(
      '#type' => 'textarea',
      '#title' => t('List of special characters to remove:'),
      '#default_value' => variable_get('rawpheno_rtransform_characters'),
      '#description' => 'Separate characters with commas',
      '#required' => TRUE,
    );  
  
    $form['fieldset_rules']['rawpheno_rtransform_replace'] = array(
      '#type' => 'textarea',
      '#title' => t('Match and replace (Match Character = Replacement):'),
      '#default_value' => variable_get('rawpheno_rtransform_replace'),
      '#description' => 'Separate match and replace pairs with commas',
      '#required' => TRUE,
    );
    
  $form['req']['#suffix'] = t('<span>&nbsp;* means field is required</span>');  
  
  // This will add a save button to this form, thus 
  // no hook_submit nor hook_validate is required.  
  return system_settings_form($form);
}


/**
 * Implements hook_validate().
 * Ensure that the match and replace entry is in match value = replace value format.
 */
function rawpheno_admin_rheaders_validate($form, &$form_state) {
  // Read string containing the match and replace rule. Since each pair is separated by comma symbol,
  // use it to split each entry. In each item, test if both match value and replace value are provided,
  // by examining the value before and after the = sign. When a pair is missing either values, send an
  // error message to admin that an entry is not in prescribed format.
  $mr = trim($form_state['values']['rawpheno_rtransform_replace']);
  
  $match_replace = explode(',', $mr);
  foreach($match_replace as $r) {
    @list($match, $replace) = explode('=', $r);
    if (empty(trim($match)) OR empty(trim($replace))) {
      form_set_error('rawpheno_rtransform_replace', t('An entry not in the required format (Match value = Replace Value) was found.'));
    }
  }
}


/**
 * Function callback: Construct form to list all active projects (with headers and users)
 * and allow admin to create a new project.
 */
function rawpheno_admin_all_projects($form, &$form_state) {
  // CONSTRUCT FORMS REQUIRED BY THIS INTERFACE.
  
  // FORM
  // Construct create project form.
  // This form will allow admin to select a project and define column headers,
  // as well as, assign active users.
  
  // Query projects that dont have column headers.
  $sql = "SELECT t1.project_id, t1.name 
          FROM {project} AS t1 LEFT JOIN pheno_project_cvterm AS t2 USING(project_id)
          WHERE t2.project_id IS NULL  
          ORDER BY t1.project_id DESC";
  
  $p = chado_query($sql);
  
  // Add form elements.  
  $form['fieldset_sel_project'] = array(
    '#type' => 'fieldset',
    '#title' => t('Create a New Project:'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  
  if ($p->rowCount() > 0) {
    // Make sure that the site has set up a project.
    $projects = $p->fetchAllKeyed();
    $options = array('0' => '---') + $projects;
    
    $form['fieldset_sel_project']['sel_project'] = array(
      '#type' => 'select',
      '#title' => t('Please select a project:'),
      '#options' => $options,
      '#default_value' => array_keys($options)[0],
      '#id' => 'admin-sel-project',
      '#description' => t('Please note that plant property column headers Plot, Entry, Rep, Name and Location are added to project by default.'),
    );

    $form['fieldset_sel_project']['add_project'] = array(
      '#type' => 'submit',
      '#value' => t('Create Project'),
      '#id' => 'admin-add-project',
    );
  }
  else {
    $form['fieldset_sel_project']['no_project'] = array(
      '#markup' => '<div class="messages warning">No project available.</div>',
    );
  }
  
  // TABLE
  // Construct table that lists all active projects in this module.
  // The table data consists of project name and summary of column headers and active users.
  
  // Get trait types array.
  $trait_type = rawpheno_function_trait_types();  
    
  // Array to hold table headers.  
  $arr_headers = array();
  
  // Array to hold table rows.
  $arr_rows = array();
  
  $empty_table_title = "";
 
  // On the bottom of the select field, list (summary of) the active projects, or projects with 
  // column headers and users assigned to it.
  $sql = "SELECT t1.name, COUNT(t2.cvterm_id) AS header_count,
            COUNT(CASE WHEN t2.type = :essential_trait THEN 1 END) AS essential_count,
            COUNT(DISTINCT uid) AS user_count, t1.project_id
          FROM {project} AS t1 INNER JOIN pheno_project_cvterm AS t2 USING (project_id)
            LEFT JOIN pheno_project_user AS t3 USING (project_id)
          GROUP BY t1.project_id
          ORDER BY t1.project_id DESC";
  
  $args = array(':essential_trait' => $trait_type['type1']);
  $projects = chado_query($sql, $args);
      
  // Page title.
  $form['all_project_info'] = array(
    '#type' => 'markup',
    '#markup' => '<h2>' . $projects->rowCount() . ' Project(s)</h2>'
  );
  
  // Table rows.
  if ($projects->rowCount() > 0) {
    $i = 0;
    foreach($projects as $p) {
      $view_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $p->project_id . '/project/manage';
      $view_cell = l('View', $view_link);
      
      array_push($arr_rows, array(($i+1), $p->name, $p->header_count, $p->essential_count, $p->user_count, $view_cell));
      $i++;
    }
  }
  else {
    $empty_table_title = t('No project available');
  }
  
  // Table headers.
  array_push($arr_headers, '-', t('Project Name'), t('Column Headers'), t('Essential Headers'), t('Active User'), t('View'));
  
  $table = theme('table', array('header' => $arr_headers, 'rows' => $arr_rows, 'empty' => $empty_table_title));
  
  // Render table.
  $form['tbl_projects'] = array(
    '#markup' => $table,
  );

  // Attach css.  
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.admin.style.css'); 

  return $form;
}


/**
 * Implements hook_validate().
 * Ensure that a project is selected when creating a new project.
 */
function rawpheno_admin_all_projects_validate($form, &$form_state) {
  $project_id = $form_state['values']['sel_project'];
  if ($project_id <= 0) {
    form_set_error('sel_project', t('No project selected. Please select a project and try again'));
  }
}


/**
 * Implements hook_sumbit().
 * Create a new project.
 */
function rawpheno_admin_all_projects_submit($form, &$form_state) {
  // When a new project is created, Plant property column headers are
  // added to a project by default. The following are the plant property headers:
  // Location, Plot Rep and Entry.
  $project_id = $form_state['values']['sel_project'];
  
  // Get trait types array.
  $trait_type = rawpheno_function_trait_types();
  
  // Query all plant property types column headers in chado.cvterm table.
  $sql = "SELECT cvterm_id 
          FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING(cv_id) 
          WHERE t1.name = 'phenotype_plant_property_types'
          ORDER BY cvterm_id ASC";

  $pp = chado_query($sql);

  // Test each plant property if it is present in the project.
  foreach($pp as $t) {
    db_insert('pheno_project_cvterm')
      ->fields(array(
        'project_id' => $project_id,
        'cvterm_id' => $t->cvterm_id,
        'type' => $trait_type['type4'])
      )
    ->execute();
  }
    
  // Redirect user to project asset management page.
  $path = url('../../admin/tripal/extension/rawphenotypes/all_projects/' . $project_id . '/project/manage');
  drupal_goto($path);
}


/**
 * Function callback: Construct form to manage project assets.
 *
 * @param $asset_id
 *   An integer containing the record id.  
 * @param $asset_type
 *   A string indicating what asset type the id is, a project, a user or a column header.
 * @param $action
 *   A string containing predefined commands performed to an asset.
 */
function rawpheno_admin_project_management($form, &$form_state, $asset_id = NULL, $asset_type = NULL, $action = NULL) {
  // Link to go back.
  $goback = '&nbsp;<a href="javascript:history.back();">Go back</a>';
  
  // Define valid action/command/operation per asset type.
  $arr_valid = array();
  $arr_valid['project']['command'] = array('manage');
  $arr_valid['project']['table'] = 'pheno_project_cvterm';
  $arr_valid['project']['id'] = 'project_id';
    
  $arr_valid['header']['command'] = array('edit', 'delete');
  $arr_valid['header']['table'] = 'pheno_project_cvterm';
  $arr_valid['header']['id'] = 'project_cvterm_id';
  
  $arr_valid['user']['command'] = array('delete');
  $arr_valid['user']['table'] = 'pheno_project_user';
  $arr_valid['user']['id'] = 'project_user_id';

  // Ensure query strings are valid. The string length check ensure that Posgres
  // will not throw Numeric value out of range PDOexception.
  if (!isset($asset_id) OR $asset_id <= 0 OR strlen($asset_id) >= 10) {
    // When project asset id is invalid.
    drupal_set_message(t('Not a valid project asset id number.') . $goback, 'error');
  }
  else { 
    // Id is valid, test if asset type is valid.
    if (array_key_exists($asset_type, $arr_valid)) {
      // Valid asset type request. Check if the given asset id and asset type
      // exists in the database before peforming any command.
      $sql = sprintf("SELECT FROM {%s} WHERE %s = :asset_id LIMIT 1", 
        $arr_valid[$asset_type]['table'], $arr_valid[$asset_type]['id']); 
      
      $args = array(':asset_id' => (int)$asset_id);     
      $prj_asset = db_query($sql, $args);
      
      if ($prj_asset->rowCount() == 1) {
        // Project asset exists in the database. Procede to command requested.
        // Determine what to do with the project asset.
        if (in_array($action, $arr_valid[$asset_type]['command'])) {
          // Command is valid. Call function that will execute the command.
          if ($asset_type == 'project') {
            // Call project function. 
            $form = rawpheno_admin_project($form, $form_state, $asset_id);
          }
          elseif ($asset_type == 'header') {
            ///////////
            // Before admin can modify or delete a column header, ensure that a header
            // is not a plant property type header, not used by another project and the header has no data associated to it.
            // In addition, Loding (scale 1-5) header is not editable.
            $trait_type = rawpheno_function_trait_types();
            $plant_property = $trait_type['type4'];
            
            // Get header properties.
            $header_asset = rawpheno_admin_header_properties($asset_id, 'full');
            
            if ($header_asset['count_data'] > 0 AND $action == 'delete') {
              // Header has data.
              drupal_set_message(t('Cannot @action this entry. Column header has data associated to it.', 
                array('@action' => $action)) . $goback, 'error');
            }
            else {
              // Call header function.
              $form = rawpheno_admin_project_headers($form, $form_state, $header_asset, $action);
            }
            ///////////
          }
          else if($asset_type == 'user') {
            ///////////
            // Ensure that user is not deleted when there is data and backup files associated.
            $user_project_asset = rawpheno_admin_user_assets($asset_id);
              
            if ($user_project_asset['project_file_count'] > 0) {
              // User is assigned to a project has backup files.
              drupal_set_message(t('Cannot @action this user. User has backup files.', 
                array('@action' => $action)) . $goback, 'error');
            }
            elseif ($user_project_asset['project_data_count'] > 0) {
              // User is assigned to a project has has data.
              drupal_set_message(t('Cannot @action this user. User is assined to a project that has data.', 
                array('@action' => $action)) . $goback, 'error');
            }
            else {
              // Call user function.
              $form = rawpheno_admin_project_users($form, $form_state, $user_project_asset, $action);
            }  
            ////////////
          }
        }
        else {
          // Not a valid command.
          drupal_set_message(t('Not a valid request.' . $goback), 'error');
        }
      }
      else {
        // Asset record not found.
        drupal_set_message(t('Project asset id number does not exist.' . $goback), 'error'); 
      }
    }
    else {
      // Asset type does not exist.
      drupal_set_message(t('Not a valid project request.' . $goback), 'error');
    }
  }

  // Attach css and javascript.  
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.admin.style.css');
  $form['#attached']['js'] = array($path . 'js/rawpheno.admin.script.js');
  
  return $form;
}


/**
 * Function callback: Construct interface to manage project asset.
 *
 * @param $project_id
 *   An integer containing the project id number.  
 */
function rawpheno_admin_project($form, &$form_state, $project_id) {
  // Construct page title and link back to projects form.
  $markup = l('Manage Projects', 'admin/tripal/extension/rawphenotypes/all_projects');
  
  $project = chado_generate_var('project', array('project_id' => $project_id));
  $markup .= '<br /><br /><h2>PROJECT: ' . $project->name . '</h2>';
  
  $form['project_info'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );
  
  // Arguments to all SQL queries below.
  $args = array(':project_id' => $project_id);
  
  // CONSTRUCT FORMS REQUIRED BY THIS INTERFACE.
  
  // FORM   
  // Construct add column header form.
  // This form will allow admin to add new column header to a project.

  // Get trait types array.
  $trait_type = rawpheno_function_trait_types();

  // Fieldset add column header form.
  $form['fieldset_trait'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add column header'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  
  // Add form elements.
  $default_values = array('count_project'    => 0, 
                          'count_data'       => 0, 
                          'txt_id'           => $project_id, 
                          'btn_trait_submit' => 'Add column header');
                          
  $form = rawpheno_admin_render_form_trait($form, $form_state, $default_values);
  
  
  // FORM
  // Construct add/user existing column headers form.
  // This form will allow admin to add multiple column headers that are predefined in this module.
  
  // Fieldset suggest existing column headers.
  $form['fieldset_existing_trait'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add existing column headers'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  
  // Add form elements.
  // Query all traits in cvterm that is not listed as among the traits in a project,
  // and suggest it to admin in table form with checboxes.
  $sql = "SELECT t2.cvterm_id, t2.name, t2.definition
          FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING(cv_id)
          WHERE t1.name = 'phenotype_measurement_types'
            AND t2.cvterm_id NOT IN (SELECT cvterm_id FROM pheno_project_cvterm WHERE project_id = :project_id)
          ORDER BY t2.cvterm_id ASC";
  
  $headers = chado_query($sql, $args);
  
  // Table rows.
  $arr_tblchkbox_rows = array();
  
  if ($headers->rowCount() > 0) {
    foreach($headers as $h) {
      $arr_tblchkbox_rows[$h->cvterm_id] = array('name' => $h->name, 'def' => $h->definition);
    }
  }
  
  // Table headers.
  $arr_tblchkbox_headers = array('name' => t('Name'), 'def' => t('Definition'));
  
  // Checkboxes and table.
  $form['fieldset_existing_trait']['tbl_existing_headers'] = array(
    '#type' => 'tableselect',
    '#header' => $arr_tblchkbox_headers,
    '#options' => $arr_tblchkbox_rows,
    '#js_select' => FALSE,
    '#prefix' => '<p>' . 
      t('The table below lists all column headers available in this module. 
      Please check the header(s) that you want to add to this project and click Add Selected Traits button.') . '</p>
      <div class="container-table-data table-data">',
    '#suffix' => '</div>',
    '#empty' => t('No column headers available'),
    '#attributes' => array(
      'id' => 'tbl-existing-headers',
      'class' => array(
        'tableheader-processed'
      ),
    ),
  );
  
  // Add submit button only there is any options available.
  if (count($arr_tblchkbox_rows) > 0) {
    $form['fieldset_existing_trait']['add_selected_trait'] = array(
      '#type' => 'submit',
      '#value' => t('Add selected headers'),
      '#validate' => array('rawpheno_admin_validate_add_existing'),
      '#submit' => array('rawpheno_admin_submit_add_existing'),
      '#limit_validation_errors' => array(array('tbl_existing_headers'), array('txt_id')),
    );
  }
  
  //FORM
  // Construct assign user to a project form.
  // This form will allow admin to assign an active user to a project. User will then be restricted
  // to upload data to project(s) he is assigned to. 
  
  // Fieldset to suggest user to a project.
  $form['fieldset_users'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add users'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  
  // Query all active users only, that is status field is set to 1.
  $sql = "SELECT uid, name, mail FROM {users}
          WHERE uid NOT IN (SELECT uid FROM {pheno_project_user} WHERE project_id = :project_id) AND status = 1
          ORDER BY name DESC";
  
  $users = db_query($sql, $args);
  
  // Table rows.
  unset($arr_tblchkbox_headers, $arr_tblchkbox_rows);
  $arr_tblchkbox_rows = array();
 
  // Table rows.
  if ($users->rowCount() > 0) {
    foreach($users as $u) {
      $arr_tblchkbox_rows[$u->uid] = array('name' => $u->name, 'email' => $u->mail);
    }  
  }
  
  // Table headers.
  $arr_tblchkbox_headers = array('name' => t('Name'), 'email' => t('Email Address'));

  // Checkboxes and table.
  $form['fieldset_users']['tbl_users'] = array(
    '#type' => 'tableselect',
    '#header' => $arr_tblchkbox_headers,
    '#options' => $arr_tblchkbox_rows,
    '#js_select' => FALSE,
    '#attributes' => array('id' => 'tbl-add-users'),
    '#prefix' => '<p>' . 
      t('The table below lists all active users in this module. 
      To assign a user to this project, check the name(s) and click Add Selected Users button.') . '</p>
      <div class="container-table-data table-data">',
    '#suffix' => '</div>',
    '#empty' => t('No users available'),
    '#attributes' => array(
      'id' => 'tbl-project-users',
      'class' => array(
        'tableheader-processed'
      ),
    ),
  );
  
  // Add submit button only there is any options available.
  if (count($arr_tblchkbox_rows) > 0) {
    $form['fieldset_users']['add_selected_user'] = array(
      '#type' => 'submit',
      '#value' => t('Add selected users'),
      '#validate' => array('rawpheno_admin_validate_add_user'),
      '#submit' => array('rawpheno_admin_submit_add_user'),
      '#limit_validation_errors' => array(array('tbl_users'), array('txt_id')),
    );
  }
  
  // DISPLAY SUMMARY OF DATA REQUIRED BY THIS INTERFACE.

  // List column headers in a project.
  // Given a project id, select all column headers belonging to that project.
  // Note the loding (scale 1-5) upright-lodged header is the only header using this
  // format (all headers use the format: name (trait rep; unit), thus a flag indicating 
  // if header is such is required to disable edit option and prevent edit functionality
  // from processing unfamiliar format. 
  $sql = "SELECT project_cvterm_id 
          FROM {cvterm} RIGHT JOIN pheno_project_cvterm USING(cvterm_id) 
          WHERE project_id = :project_id ORDER BY type, name ASC";
  
  $headers = chado_query($sql, $args);
  
  // List active users in a project.
  $sql = "SELECT project_user_id, uid, name, mail FROM {users} RIGHT JOIN {pheno_project_user} USING(uid)
          WHERE project_id = :project_id
          ORDER BY name DESC";
  
  $users = db_query($sql, $args);
  
  // Tabs to show either the project column headers table or project active users table.
  $form['nav_tabs'] = array(
    '#markup' => '<div id="nav-tabs">
                    <ul>
                      <li class="active-tab">' . $headers->rowCount() . ' Column Headers</li>
                      <li>' . $users->rowCount() . ' Active Users</li>
                    </ul>
                  </div>'
  );
  
  // Array to hold table headers.
  $arr_headers = array();
  
  // Array to hold table rows.
  $arr_rows = array();
  
  // Array to hold table properties.
  $arr_tbl_args = array(
    'header' => '',
    'rows' => '',
    'sticky' => FALSE,
    'empty' => '',
    'attributes' => array('id' => ''),
  );
  
  
  // TABLE
  // Construct table that lists all column headers specific to a project.
  // Options to edit and delete items give admin record management functionality.
  // NOTE: Delete option will not physically delete a record in cvterm table.
  //       When a header is deleted, it is removed from the project only.
  
  // Table rows.
  if ($headers->rowCount() > 0) {  
    $i = 0;
    foreach($headers as $h) {
      // Get header information.
      $header_asset = rawpheno_admin_header_properties($h->project_cvterm_id);
      
      // Edit link.
      $edit_cell = '-';
      if ($header_asset['name'] != 'Lodging (Scale: 1-5) upright - lodged') {
        $edit_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $h->project_cvterm_id . '/header/edit';
        $edit_cell = l('Edit', $edit_link);
      }
      
      // Delete link. 
      $del_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $h->project_cvterm_id . '/header/delete';
      $del_cell = l('Delete', $del_link, array('attributes' => array('class' => 'link-del')));
      
      // No edit and delete when column header is of type plantproperty.
      if ($header_asset['type'] == $trait_type['type4']) {
        $edit_cell = $del_cell = '-';
      }
      
      $class = ($header_asset['type'] == $trait_type['type1']) ? 'essential-trait' : 'non-essentialtrait';
      array_push($arr_rows, array(($i+1), $header_asset['name'], $header_asset['r_version'], array('data' => strtoupper($header_asset['type']), 'class' => array($class)), $edit_cell, $del_cell));

      $i++;
    }
  }
  else {
    // When no records found, update the empty table property.
    $arr_tbl_args['empty'] = t('No column headers in this project');
  }
  
  // Table headers.
  array_push($arr_headers, '-', t('Column Header <small>(unit)</small>'), t('R Friendly'), t('Type'), t('Edit'), t('Delete'));
  
  // Add Name column header to the row array.
  array_push($arr_rows, array((count($arr_rows) + 1), 'Name', 'Name', 'PLANTPROPERTY', '-', '-'));
  
  // Update table properties.
  $arr_tbl_args['header'] = $arr_headers;
  $arr_tbl_args['rows'] = $arr_rows;
  $arr_tbl_args['attributes']['id'] = 'tbl-project-headers';
  
  // Render the table element.
  $form['tbl_project_headers'] = array(
    '#markup' => theme('table', $arr_tbl_args),
    '#prefix' => '<div id="container-prj-hdr" class="container-table-data tab-data">',
    '#suffix' => '</div>',
  );
  
  
  // TABLE
  // Construct table that lists all active users to a project.
  // Options to delete items give admin record management functionality.
  // NOTE: Delete option will not physically delete a record in users table.
  //       When a user is deleted, it is removed from the project only.  
  
  $arr_rows = $arr_headers = array();
  
  // Table rows
  if ($users->rowCount() > 0) {
    $i = 0;
    foreach($users as $u) {
      // Delete link.
      $del_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $u->project_user_id . '/user/delete';
      $del_cell = l('Delete', $del_link, array('attributes' => array('class' => 'link-del')));
      
      array_push($arr_rows, array(($i+1), $u->name, $u->mail, $del_cell));
      
      $i++;
    }
  }
  else {
    // When no records found, update the empty table property.
    $arr_tbl_args['empty'] = t('No active user in this project');
  }
  
  // Table headers.
  array_push($arr_headers, '-', t('Name'), t('Email Address'), t('Delete'));
  
  // Update table properties.
  $arr_tbl_args['header'] = $arr_headers;
  $arr_tbl_args['rows'] = $arr_rows;
  $arr_tbl_args['attributes']['id'] = 'tbl-project-users';
  
  // Render the table element.
  $form['tbl_project_users'] = array(
    '#markup' => theme('table', $arr_tbl_args),
    '#prefix' => '<div id="container-prj-usr" class="container-table-data tab-data">',
    '#suffix' => '</div>',
  );
  
  return $form;
}


/**
 * Function callback: Construct form to edit column header and handle deletion.
 *
 * @param $header_asset
 *   An array containing information about a column header.
 * @param $action
 *   A string containing the process to be carried out to a header.
 */
function rawpheno_admin_project_headers($form, $form_state, $header_asset, $action) {
  if ($action == 'edit') {
    if ($header_asset['name'] == 'Lodging (Scale: 1-5) upright - lodged') {
      // Is lodging header.
      $goback = '&nbsp;<a href="javascript:history.back();">Go back</a>';
      drupal_set_message(t('Cannot edit @name.', array('@name' => $header_asset['name'])) . $goback, 'error');
    }
    else {
      // Other headers.
      // CONSTRUCT EDIT FORM.
      
      // FORM 
      // Add a link to allow administrator to go back to the list of traits in a project.
      $form['back_link'] = array(
        '#type' => 'markup',
        '#markup' => l('Manage Projects', 'admin/tripal/extension/rawphenotypes/all_projects') . ' | ' . l(t('Return to project column headers table') ,'admin/tripal/extension/rawphenotypes/all_projects/' . $header_asset['in_project_id'] . '/project/manage') . '<br /><br />',
      );
      
      $form['fieldset_trait'] = array(
        '#type' => 'fieldset',
        '#title' => t('Eidt Column Header:'),
        '#collapsed' => FALSE,
        '#collapsible' => TRUE,
      );
      
      // Trait name field.
      // Extract the trait rep value, trait unit and the trait name.
      // NOTE: this can only process header following the format: trait name (trait rep; unit)
      $trait_rep = $trait_unit = '';
      // Extract the text value inside the parenthesis (unit part).
      $t = preg_match("/.*\(([^)]*)\)/", $header_asset['name'], $match);
      $u = (isset($match[1])) ? $match[1] : '';
    
      // Extract information in a unit only when there is a unit in the first place.
      // Get the Trait rep (eg R1, R7, 1st) and measurement unit (eg cm, days) values.
      $reps = rawpheno_function_trait_reps();
      
      if ($t) {
        // Split the information and see if either trait rep or unit is present.
        $p = explode(' ', $u);
        if (count($p) > 1) {
          // Has trait rep and unit.
          list($trait_rep, $trait_unit) = $p;
        }
        else {
          if (in_array(trim($u, ';'), $reps)) {
            // Just the rep no unit.
            $trait_rep = $u;
            $trait_unit = '';
          }
          else {
            // Just the unit no trait rep.
            $trait_rep = '';
            $trait_unit = $u;
          }
        }
      }
      
      // When niether is present, use the default value of the trait rep and unit (null). 
      // Trait name without the unit part.
      $trait_name = preg_replace('/\(.*/', ' ', $header_asset['name']);

      // Add form elements.
      $default_values = array('count_project'        => $header_asset['count_project'],
                              'count_data'           => $header_asset['count_data'],
                              'txt_id'               => $header_asset['cvterm_id'],
                              'txt_trait_name'       => trim($trait_name),
                              'sel_trait_rep'        => trim($trait_rep),
                              'sel_trait_unit'       => trim($trait_unit),
                              'txt_trait_definition' => $header_asset['definition'],
                              'txt_trait_rfriendly'  => $header_asset['r_version'],
                              'sel_trait_type'       => $header_asset['type'],
                              'btn_trait_submit'     => 'Save');      
      
      $form = rawpheno_admin_render_form_trait($form, $form_state, $default_values);
      
      // TABLE
      // Construct a table, as a summary, showing information about the header.
      
      $markup = '<h2>SUMMARY: ' . $header_asset['name'] . '</h2> PROJECT: ' . $header_asset['in_project_name'];

      // Array to hold table properties.
      $arr_tbl_args = array(
        'header' => array('-', t('Column Header'), t('R Friendly'), t('Definition'), t('Type')),
        'rows' => array(array(1, $header_asset['name'], $header_asset['r_version'], $header_asset['definition'], strtoupper($header_asset['type']))),
        'sticky' => FALSE,
        'attributes' => array('id' => 'tbl_header_summary'),
      );

      // Render the table element.
      $form['tbl_project_headers'] = array(
        '#markup' => $markup . theme('table', $arr_tbl_args),
      );
        
      return $form;
    }
  }
  elseif ($action == 'delete') {
    $path = url('../../admin/tripal/extension/rawphenotypes/all_projects/' . $header_asset['in_project_id'] . '/project/manage');

    // Admin wants to delete/remove the trait from the project.
    db_delete('pheno_project_cvterm')
      ->condition('cvterm_id', $header_asset['cvterm_id'])
      ->condition('project_id', $header_asset['in_project_id'])
      ->execute();

    drupal_goto($path);
  }
}


/**
 * Implements hook_validate().
 * Validate entries in add column headers form.
 */
function rawpheno_admin_project_management_validate($form, &$form_state) {
  // Get the submit button that triggered the submit action. Since this is a general hook_validate()
  // of the entire form, limit the proces only to submit button in from add column header and save column header.
  // When the submit action is determined, perform basic check of ensuring that user is not using cvterm name
  // that is in the database already.
  $btn_submit = $form_state['triggering_element']['#value'];
   
  if ($btn_submit == 'Add column header' OR $btn_submit == 'Save') {
    $asset_id = $form_state['values']['txt_id'];  
    
    // Construct the trait name based on given name, rep and unit.
    $trait_name = rawpheno_admin_construct_trait_name(
      array('name' => $form_state['values']['txt_trait_name'],
            'rep'  => $form_state['values']['sel_trait_rep'],
            'unit' => $form_state['values']['sel_trait_unit'])
    );
    
    if ($btn_submit == 'Add column header') {
      // Before adding the cvterm, ensure it is not present in cvterm table.
      $found_cvterm = tripal_get_cvterm(array('name' => $trait_name, 'cv_id' => array('name' => 'phenotype_measurement_types')));
      
      if (isset($found_cvterm->cvterm_id) AND $found_cvterm->cvterm_id > 0) {
        form_set_error('txt_trait_name', t('The column header name exists in the database. Please use a different name.'));
        form_set_error('sel_trait_rep');
        form_set_error('sel_trait_unit');
      }
    }
    else {
      // When renaming a field in edit header, ensure that the new or modified name does not exist in cvterm table.
      $sql = "SELECT cvterm_id FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING(cv_id)
              WHERE t1.name = 'phenotype_measurement_types' AND t2.name = :name AND t2.cvterm_id <> :this_cvterm_id";
      
      $args = array(':name' => $trait_name, ':this_cvterm_id' => $asset_id);
      $found = chado_query($sql, $args);
      if ($found->rowCount() > 0) {
        form_set_error('txt_trait_name', t('The column header name exists in the database. Please use a different name.'));
        form_set_error('sel_trait_rep');
        form_set_error('sel_trait_unit');
      }
    }
  }
}


/**
 * Implements hook_submit().
 * Add project assets (headers and users).
 */
function rawpheno_admin_project_management_submit($form, &$form_state) {
  // Get the submit button that triggered the submit action. Since this is a general hook_submit()
  // of the entire form, limit the proces only to submit button in from add column header and save column header.
  $btn_submit = $form_state['triggering_element']['#value'];

  $asset_id = $form_state['values']['txt_id'];
  
  if ($btn_submit == 'Add column header' OR $btn_submit == 'Save') {
    // Trait definition
    $trait_def = $form_state['values']['txt_trait_def'];

    // Construct the trait name based on given name, rep and unit.
    $trait_name = rawpheno_admin_construct_trait_name(
      array('name' => $form_state['values']['txt_trait_name'],
            'rep' => $form_state['values']['sel_trait_rep'],
            'unit'=> $form_state['values']['sel_trait_unit'])
    );
    
    if ($btn_submit == 'Add column header') {
      // Uses project id.
      // Insert a cv term of type phenotype_measurement_types.
      $arr_terms_param = array('id' => 'tripal:' . $trait_name,
                               'name' => $trait_name,
                               'definition' => $trait_def,
                               'cv_name' => 'phenotype_measurement_types');
      
      $cvterm_obj = tripal_insert_cvterm($arr_terms_param);
      
      // R Friendly version of the Header name.
      // When supplied, use it, otherwise transform the name to R friendly.
      $rfriendly = (empty($form_state['values']['txt_trait_rfriendly']))
        ? rawpheno_function_make_r_compatible($trait_name)
        : $form_state['values']['txt_trait_rfriendly'];
    
      // CV of type R compatible column header.
      $cv_rfriendly = tripal_get_cv(array('name' => 'r_compatible_column_header'));

      // Add a cvterm prop to store the R friendly version.
      $values = array(
        'cvterm_id' => $cvterm_obj->cvterm_id,
        'type_id' => $cv_rfriendly->cv_id,
        'value' => $rfriendly,
        'rank' => 0
      );

      chado_insert_record('cvtermprop', $values);

      // Add entry to project cvterms table.
      $trait_type = $form_state['values']['sel_trait_type'];
      db_insert('pheno_project_cvterm')
        ->fields(array(
          'project_id' => $asset_id,
          'cvterm_id' => $cvterm_obj->cvterm_id,
          'type' => $trait_type)
        )
        ->execute();
        
      drupal_set_message(t('You have successfully added trait to this project.'), 'status'); 
    }
    else {
      // Uses cvterm id number.
      // Update cvterm record (name and definition).
      $umatch = array('cvterm_id' => $asset_id);
      $uvalues = array('name' => $trait_name, 'definition' => $trait_def);
      chado_update_record('cvterm', $umatch, $uvalues);      

      // Update cvtermprop (rfriendly version).
      $cv_rfriendly = tripal_get_cv(array('name' => 'r_compatible_column_header'));
      $rfriendly = (empty($form_state['values']['txt_trait_rfriendly']))
        ? rawpheno_function_make_r_compatible($trait_name)
        : $form_state['values']['txt_trait_rfriendly'];
    
      $umatch = array('cvterm_id' => $asset_id, 'type_id' => $cv_rfriendly->cv_id);
      $uvalues = array('value' => $rfriendly);
      chado_update_record('cvtermprop', $umatch, $uvalues);      
    
      $trait_type = $form_state['values']['sel_trait_type'];

      db_update('pheno_project_cvterm')
        ->fields(array('type' => $trait_type))
        ->condition('cvterm_id', $asset_id, '=')
        ->execute();

      drupal_set_message(t('You have successfully updated a column header in this project.'), 'status');
    }
  }
}


/**
 * Function callback: Delete a user from a project.
 * Note: The record of user is not physically deleted from the database.
 * When deleting a user, it is only removed from the assined project.
 *
 * @param $user_project_asset
 *   An array containing data and file information about a project the user is assigned.
 */
function rawpheno_admin_project_users($form, $form_state, $user_project_asset, $action) {
  if ($action == 'delete') {
    $path = url('../../admin/tripal/extension/rawphenotypes/all_projects/' . $user_project_asset['project_id'] . '/project/manage');

    db_delete('pheno_project_user')
      ->condition('project_user_id', $user_project_asset['project_user_id'])
      ->execute();

    // Redirect user.
    drupal_goto($path);
  }
}


/**
 * Function callback: Construct a form to either modify or add a column header.
 *
 * @param $default
 *   An array containing default values of each form elements.
 */
function rawpheno_admin_render_form_trait($form, &$form_state, $default) {
  // Get trait types array.
  $a = rawpheno_function_trait_types();
  // In this trait types array, remove the plant property option.
  // Allow only to add essential, optional or subset column header types.
  unset($a['type4']);
  
  // Get the values and use it as both key and value.
  $t = array_values($a);
  $trait_type = array_combine($t, $t); 

  // Determine if the header has data or is used in another project.
  if ($default['count_data'] > 0 OR $default['count_project'] > 0) {
    $form['fieldset_trait']['warning'] = array(
      '#markup' => '<div class="messages warning">This column header has data associated to it or is used in another project.</div>',
    );
    
    $disabled = TRUE;
  }
  else {
    $disabled = FALSE;
  }
  
  // Project id the trait is in.
  $form['fieldset_trait']['txt_id'] = array(
    '#type' => 'hidden',
    '#value' => $default['txt_id'],
  );
  
  // Trait name field.
  $form['fieldset_trait']['txt_trait_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name:'),
    '#description' => t('A Concise human-readable name or label for the column header'),
    '#default_value' => isset($default['txt_trait_name']) ? $default['txt_trait_name'] : '',
    '#disabled' => $disabled,
    '#required' => TRUE,
  );  

  // Trait rep field. Default to none.
  // Note: Trait Rep to call 1st, 2nd in unit is not final.
  // Note: The list might change in the future.
  $reps = rawpheno_function_trait_reps();
  $reps_val = array();
  foreach($reps as $r) {
    array_push($reps_val, $r . ';');
  }
  $trait_rep = array('' => 'None') + array_combine($reps_val, $reps);

  $form['fieldset_trait']['sel_trait_rep'] = array(
    '#type' => 'select',
    '#title' => t('Trait Rep:'),
    '#options' => $trait_rep,
    '#default_value' => isset($default['sel_trait_rep']) ? $default['sel_trait_rep'] : reset($trait_rep),
    '#disabled' => $disabled,
  );  
  
  // Trait unit field. Default to none.
  // Query available units in chado cvterm of type phenotype_measurement_unit.
  $sql = "
    SELECT t2.name, t2.name || ' : ' || t2.definition
    FROM {cv} AS t1 INNER JOIN {cvterm} AS t2 USING (cv_id) 
    WHERE t1.name = 'phenotype_measurement_units' 
    ORDER BY t2.name ASC";
    
  $unit = chado_query($sql)
    ->fetchAllKeyed();
  
  $form['fieldset_trait']['sel_trait_unit'] = array(
    '#type' => 'select',
    '#title' => t('Unit:'),
    '#options' => array('' => 'None') + $unit,
    '#default_value' => isset($default['sel_trait_unit']) ? $default['sel_trait_unit'] : reset($unit),
    '#disabled' => $disabled,
  ); 
  
  // R Friendly field.
  $r_version = isset($default['txt_trait_rfriendly']) ? ' (' . $default['txt_trait_rfriendly'] . ')' : '';
  
  $form['fieldset_trait']['txt_trait_rfriendly'] = array(
    '#type' => 'textfield',
    '#title' => t('R Friendly:@r_version', array('@r_version' => $r_version)),
    '#description' => t('Leave this field blank to let system generate R Friendly version'),
    '#default_value' => '',
    '#disabled' => $disabled,
  );   
  
  if (isset($default['txt_trait_rfriendly']) AND !empty($default['txt_trait_rfriendly'])) {   
    // When editing header, add this hidden field containing the original R version value prior to saving.
    // When user decides to provide an alternative r version then save it, otherwise save the value of this
    // hidden field. 
    $form['fieldset_trait']['txt_trait_rfriendly_val'] = array(
      '#type' => 'hidden',
      '#default_value' => $default['txt_trait_rfriendly'],
    );   
  }
  
  // Trait definition field.
  $form['fieldset_trait']['txt_trait_def'] = array(
    '#type' => 'textfield',
    '#title' => t('Definition:'),
    '#description' => t('A human-readable text definition'),
    '#required' => FALSE,
    '#default_value' => isset($default['txt_trait_definition']) ? $default['txt_trait_definition'] : '',
    '#disabled' => $disabled,
  );  
  
  // Trait is essential field. Default to unchecked.
  $form['fieldset_trait']['sel_trait_type'] = array(
    '#type' => 'select',
    '#title' => t('Type:'),
    '#description' => t('Use essential to ensure this header must exists in the spreadsheet file'),
    '#options' => $trait_type,
    '#default_value' => isset($default['sel_trait_type']) ? $default['sel_trait_type'] : reset($trait_type),
    '#disabled' => FALSE,
  );  

  // Save trait button.
  $form['fieldset_trait']['btn_trait_subtmit'] = array(
    '#type' => 'submit',
    '#value' => t('@save_or_add', array('@save_or_add' => $default['btn_trait_submit'])),
    '#suffix' => '<span>&nbsp;* means field is required</span>',
  );
  
  return $form;      
}


/**
 * Function to construct a trait given a name, trait rep and unit.
 *
 * @param $trait
 *   An array containing name, trait and unit. 
 * 
 * @return 
 *   A string containing containing the column header in name (trait rep; unit) format.
 */
function rawpheno_admin_construct_trait_name($trait) {
  $unit = '';
  
  if (!empty($trait['unit']) OR !empty($trait['rep'])) {
    $u = (empty($trait['rep'])) ? '' : $trait['rep'] . ' ';
    if (empty($trait['unit'])) {
      $u = trim($u);
    }
    
    $unit = '(' . $u . $trait['unit'] . ')';
  }  

  $name = ucfirst($trait['name'] . ' ' . rtrim($unit));  
  
  return trim($name);
}


/**
 * Function callback: Save values in add existing column headers form.
 */
function rawpheno_admin_submit_add_existing($form, &$form_state) {
  // Uses project id
  $project_id = $form_state['values']['txt_id'];
  $trait_type = rawpheno_function_trait_types();

  // The type of header is set to OPTIONAL trait.
  foreach(array_filter($form_state['values']['tbl_existing_headers']) as $m) {
    db_insert('pheno_project_cvterm')
      ->fields(array(
        'project_id' => $project_id,
        'cvterm_id' => $m,
        'type' => $trait_type['type2']))
      ->execute();
  }

  drupal_set_message(t('You have successfully added column headers to this project.'), 'status');  
}


/**
 * Function callback: Validate add existing column headers.
 */
function rawpheno_admin_validate_add_existing($form, &$form_state) {
  if (count($form_state['values']['tbl_existing_headers']) <= 0) {
    form_set_error('tbl_existing_headers', t('No column headers selected.'));
  }
}
  
/**
 * Function callback: Save values in add user form.
 */
function rawpheno_admin_submit_add_user($form, $form_state) {
  // Uses project id
  $project_id = $form_state['values']['txt_id'];

  // Assign users to project, who will be authorized to submit project specific data.   
  foreach(array_filter($form_state['values']['tbl_users']) as $u) {
    db_insert('pheno_project_user')
      ->fields(array(
        'project_id' => $project_id,
        'uid' => $u))
      ->execute();
  }

  drupal_set_message(t('You have successfully added users to this project.'), 'status');  
}


/**
 * Function callback: Validate add user.
 */
function rawpheno_admin_validate_add_user($form, &$form_state) {
  if (count($form_state['values']['tbl_users']) <= 0) {
    form_set_error('tbl_users', t('No user selected.'));
  }
}


/**
 * Function get properties of project the user is assigned.
 *
 * @param $asset_id
 *   An integer containing the project-user id number.
 *
 * @return
 *   An array containing data and file information of a project the user is assigned.
 */
function rawpheno_admin_user_assets($asset_id) {
  // Array to hold user assets.
  $arr_project_assets = array();

  $sql = "SELECT project_id, uid, project_user_id 
          FROM {pheno_project_user} WHERE project_user_id = :project_user_id LIMIT 1";
            
  $args = array(':project_user_id' => $asset_id);
  $user = db_query($sql, $args)
    ->fetchObject();

  $arr_project_assets['user_id'] = $user->uid;
  $arr_project_assets['project_id'] = $user->project_id;
  $arr_project_assets['project_user_id'] = $user->project_user_id;
  
  // Given the project id this user is assined, test if it has any backup files.
  $sql = "SELECT COUNT(file_id) FROM {pheno_backup_file} WHERE project_user_id = :project_user_id";
  $file_count = db_query($sql, $args)
    ->fetchField();

  $arr_project_assets['project_file_count'] = $file_count;

  // Given the project id this user is assined, test if it has data.
  $sql = "SELECT COUNT(plant_id) FROM {pheno_plant_project}
          WHERE project_id = (SELECT project_id FROM {pheno_project_user} WHERE project_id = :project_id AND uid = :user_id)";
  
  $args = array(':project_id' => $user->project_id, ':user_id' => $user->uid);
  $data_count = db_query($sql, $args)
    ->fetchField();

  $arr_project_assets['project_data_count'] = $data_count;  

  return $arr_project_assets;
}


/**
 * Function to get all properties of a column header.
 *
 * @param $asset_id
 *   An integer containing record id number of a given column header.
 *
 * @return
 *   An array containing all properties (project, name, data, etc.) of a column header.
 */
function rawpheno_admin_header_properties($asset_id, $dataset = NULL) {
  // Array to hold properties.
  $arr_properties = array();

  // Get project information and header type.
  $sql = "SELECT t1.project_id, t1.name, t2.cvterm_id, t2.type 
          FROM {project} AS t1 INNER JOIN pheno_project_cvterm AS t2 USING(project_id)
          WHERE t2.project_cvterm_id = :record_id LIMIT 1";
  
  $args = array(':record_id' => $asset_id);
  $h = chado_query($sql, $args)
    ->fetchObject();
  
  $arr_properties['in_project_id'] = $h->project_id;
  $arr_properties['in_project_name'] = $h->name;
  $arr_properties['cvterm_id'] = $h->cvterm_id;
  $arr_properties['type'] = $h->type;

  $args = array(':project_id' => $h->project_id, ':cvterm_id' => $h->cvterm_id);         
  
  // Other properties.
  $sql = "SELECT t1.name, t1.definition, t2.value AS rversion 
          FROM {cvterm} AS t1 INNER JOIN {cvtermprop} AS t2 USING(cvterm_id) 
          WHERE t1.cvterm_id = (SELECT cvterm_id FROM pheno_project_cvterm WHERE project_id = :project_id AND cvterm_id = :cvterm_id LIMIT 1)
          LIMIT 1";
  
  $h = chado_query($sql, $args)
    ->fetchObject();              

  $arr_properties['name'] = $h->name;
  $arr_properties['definition'] = $h->definition;
  $arr_properties['r_version'] = $h->rversion;
  
  // Request full dataset including basic stats about the header.
  if ($dataset == 'full') {  
    // Count data associated to column header.
    $sql = "SELECT COUNT(type_id) AS data_count FROM {pheno_measurements} 
            WHERE type_id = :cvterm_id AND plant_id IN (SELECT plant_id FROM {pheno_plant_project} WHERE project_id = :project_id)";         
  
    $h = db_query($sql, $args)
      ->fetchObject();
  
    $arr_properties['count_data'] = $h->data_count;

    // Count the projects this same column header is being used.
    $sql = "SELECT COUNT(project_id) AS project_count 
            FROM {pheno_project_cvterm} WHERE project_id <> :project_id AND cvterm_id = :cvterm_id"; 
  
    $h = db_query($sql, $args)
      ->fetchObject();
  
    $arr_properties['count_project'] = $h->project_count;
  }
  
  return $arr_properties;
}