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
  $breadcrumbs = array();
  $breadcrumbs[] = l('Home', '');
  $breadcrumbs[] = l('Rawphenotypes', '/admin/tripal/extension/rawphenotypes');
  $breadcrumbs[] = l('Page Configurations', '/admin/tripal/extension/rawphenotypes/rawpheno_config');

  drupal_set_breadcrumb($breadcrumbs);

  // Set the title of this page. This line will create a page title on upper left hand
  // corner of the page preceding the tabs.
  drupal_set_title('Page Configurations');

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

  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.admin.style.css');

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
  $breadcrumbs = array();
  $breadcrumbs[] = l('Home', '');
  $breadcrumbs[] = l('Rawphenotypes', '/admin/tripal/extension/rawphenotypes');
  $breadcrumbs[] = l('Define R Transformation Rules', '/admin/tripal/extension/rawphenotypes/rawpheno_rheaders');

  drupal_set_breadcrumb($breadcrumbs);

  // Set the title of this page. This line will create a page title on upper left hand
  // corner of the page preceding the tabs.
  drupal_set_title('Define R Transformation Rules');

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
      '#description' => t('Separate words with commas'),
      '#required' => TRUE,
    );

    $form['fieldset_rules']['rawpheno_rtransform_characters'] = array(
      '#type' => 'textarea',
      '#title' => t('List of special characters to remove:'),
      '#default_value' => variable_get('rawpheno_rtransform_characters'),
      '#description' => t('Separate characters with commas'),
      '#required' => TRUE,
    );

    $form['fieldset_rules']['rawpheno_rtransform_replace'] = array(
      '#type' => 'textarea',
      '#title' => t('Match and replace (Match Character = Replacement):'),
      '#default_value' => variable_get('rawpheno_rtransform_replace'),
      '#description' => t('Separate match and replace pairs with commas'),
      '#required' => TRUE,
    );

  $form['req']['#suffix'] = t('<span>&nbsp;* means field is required</span>');

  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.admin.style.css');


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
  // use it to split and process each entry. In each item, test if both match value and replace value are provided,
  // by examining the value before and after the = sign. When a pair is missing either values, send an
  // error message to admin that an entry is not in the prescribed format.
  $mr = trim($form_state['values']['rawpheno_rtransform_replace']);

  $match_replace = explode(',', $mr);
  foreach($match_replace as $r) {
    @list($match, $replace) = explode('=', $r);
    if (empty(trim($match)) OR empty(trim($replace))) {
      form_set_error('rawpheno_rtransform_replace', t('An entry not in the required format (Match value = Replace value) was found.'));
    }
  }
}


/**
 * Function callback: Construct form to list all active projects (with headers and users)
 * and allow admin to create a new project.
 */
function rawpheno_admin_all_projects($form, &$form_state) {
  $breadcrumbs = array();
  $breadcrumbs[] = l('Home', '');
  $breadcrumbs[] = l('Rawphenotypes', '/admin/tripal/extension/rawphenotypes');
  $breadcrumbs[] = l('Manage Projects', '/admin/tripal/extension/rawphenotypes/all_projects');

  drupal_set_breadcrumb($breadcrumbs);

  // Set the title of this page. This line will create a page title on upper left hand
  // corner of the page preceding the tabs.
  drupal_set_title('Manage Projects');

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
    '#collapsed' => FALSE,
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
      '#description' => t('Please note that column headers Plot, Entry, Rep, Name, Location and Planting Date (date) are added to project by default.'),
    );

    $form['fieldset_sel_project']['add_project'] = array(
      '#type' => 'submit',
      '#value' => t('Create Project'),
      '#id' => 'admin-add-project',
    );
  }
  else {
    $form['fieldset_sel_project']['no_project'] = array(
      '#markup' => '<div class="messages warning">No projects available.</div>',
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

  $sql = "SELECT
            t1.name,
            COUNT(DISTINCT t2.cvterm_id) AS header_count,
            COUNT(DISTINCT t3.cvterm_id) AS essential_count,
            COUNT(DISTINCT t4.uid) AS user_count,
            t1.project_id
          FROM
            {project} AS t1
            INNER JOIN pheno_project_cvterm AS t2 USING (project_id)
            INNER JOIN pheno_project_cvterm AS t3 USING (project_id)
            INNER JOIN pheno_project_user AS t4 USING (project_id)
          WHERE
            t1.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
            AND t2.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
            AND t3.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
            AND t3.type = 'essential'
            AND t4.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
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
      $name_cell = l($p->name, $view_link);

      // Warn user that the project has no essential trait. Project must have at least 1 essential trait
      // list of column headers before it can process and store phenotypic data. The same for user.
      $warn_header = ($p->essential_count > 0)
        ? ''
        : '&nbsp;&nbsp;<img src="../../../../misc/message-16-warning.png" title="Project has no essential column header" alt="Project has no essential column header" />';

      $warn_user = ($p->user_count > 0)
        ? ''
        : '&nbsp;&nbsp;<img src="../../../../misc/message-16-warning.png" title="Project has no assigned user" alt="Project has no assigned user" />';


      // + 1 to account for Name column header.
      array_push($arr_rows, array(($i+1), $name_cell, ($p->header_count + 1), $p->essential_count . $warn_header, $p->user_count . $warn_user, $view_cell));
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

  // Plus and Planting Date and make it essential trait. Planting date is important
  // as it is used when generating the heatmap.
  $project_id = $form_state['values']['sel_project'];

  // Get trait types array.
  $trait_type = rawpheno_function_trait_types();

  // Get Planting Date cvterm id.
  $planting_date = tripal_get_cvterm(array('name' => 'Planting Date (date)',
                                     'cv_id' => array('name' => 'phenotype_measurement_types')));

  db_insert('pheno_project_cvterm')
    ->fields(array(
      'project_id' => $project_id,
      'cvterm_id' => $planting_date->cvterm_id,
      'type' => $trait_type['type1'])
    )
  ->execute();

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

  // Make who created the project as the default user.
  db_insert('pheno_project_user')
    ->fields(array(
      'project_id' => $project_id,
      'uid' => $GLOBALS['user']->uid)
    )
  ->execute();

  if ($GLOBALS['user']->uid != 1) {
    // If the user is not the superadmin, add as well.
    db_insert('pheno_project_user')
      ->fields(array(
        'project_id' => $project_id,
        'uid' => 1)
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

  $arr_valid['envdata']['command'] = array('delete');
  $arr_valid['envdata']['table'] = 'pheno_environment_data';
  $arr_valid['envdata']['id'] = 'environment_data_id';

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
            $header_asset = rawpheno_function_header_properties($asset_id, 'full');

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
              drupal_set_message(t('Cannot @action this user. User is assigned to a project that has data.',
                array('@action' => $action)) . $goback, 'error');
            }
            elseif ($user_project_asset['user_id'] < 2) {
              // User is administrator.
              drupal_set_message(t('Cannot @action user. The user is administrator of this site.',
                array('@action' => $action)) . $goback, 'error');
            }
            else {
              // Call user function.
              $form = rawpheno_admin_project_users($form, $form_state, $user_project_asset, $action);
            }
            ////////////
          }
          else if($asset_type == 'envdata') {
            ////////////
            if ($asset_id > 0 && $action == 'delete') {
              // Locate file. Ensure that file to be deleted exists.
              $envdata_project_asset = rawpheno_admin_envdata_asset($asset_id);

              if ($envdata_project_asset) {
                // Call user function.
                $form = rawpheno_admin_project_envdata($form, $form_state, $envdata_project_asset, $action);
              }
              else {
                // User is administrator.
                drupal_set_message(t('Cannot @action Environment Data File. File does not exist.',
                  array('@action' => $action)) . $goback, 'error');
              }
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
  $breadcrumbs = array();
  $breadcrumbs[] = l('Home', '');
  $breadcrumbs[] = l('Rawphenotypes', '/admin/tripal/extension/rawphenotypes');
  $breadcrumbs[] = l('Manage Projects', '/admin/tripal/extension/rawphenotypes/all_projects');

  drupal_set_breadcrumb($breadcrumbs);

  // Set the title of this page. This line will create a page title on upper left hand
  // corner of the page preceding the tabs.
  drupal_set_title('Manage Project Assets');

  // Construct page title and link back to projects form.
  $markup = l('Go Back to Projects Table', 'admin/tripal/extension/rawphenotypes/all_projects');

  $project = chado_generate_var('project', array('project_id' => $project_id));
  $markup .= '<h2>PROJECT: ' . $project->name . '</h2>';

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
  $sql = "SELECT t2.cvterm_id, t2.name
          FROM
            {cv} AS t1
            INNER JOIN {cvterm} AS t2
            USING(cv_id)
          WHERE
            t1.name = 'phenotype_measurement_types'
            AND t2.cvterm_id NOT IN (SELECT cvterm_id FROM pheno_project_cvterm WHERE project_id = :project_id)
          ORDER BY t2.cvterm_id ASC";

  $headers = chado_query($sql, $args);

  // Table rows.
  $arr_tblchkbox_rows = array();

  if ($headers->rowCount() > 0) {
    foreach($headers as $h) {
      $arr_tblchkbox_rows[$h->cvterm_id] =
        array(
          'name' => $h->name,
          // Add select box.
          'traittype' =>
            array('data' =>
              array(
                '#type' => 'checkbox',
                '#title' => 'Yes',
                '#options' => array(0, 1),
                '#default_value' => 0,
                '#name' => 'traittype-' . $h->cvterm_id
              )
            )
        );
    }
  }

  // Table headers.
  $arr_tblchkbox_headers = array('traittype' => t('Is Essential?'), 'name' => t('Name'));

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
    '#suffix' => '</div><p>' .
      t('Check <span class="essential-trait">IS ESSENTIAL?</span> to ensure the header must exists in the spreadsheet file')
      . '</p>',
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
      '#limit_validation_errors' => array(
        array('tbl_existing_headers'),
        array('txt_id')
      ),
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


  // Add seach field to filter the list of name.
  $form['fieldset_users']['txt_autocomplete_user'] = array(
    '#title' => 'Name :',
    '#type' => 'textfield',
    '#maxlength' => 50,
    '#size' => 130,
    '#autocomplete_path' => 'admin/tripal/extension/rawphenotypes/username/' . $project_id,
    '#description' => t('Type the name or username of the user')
  );

  $form['fieldset_users']['add_selected_user'] = array(
     '#type' => 'submit',
     '#value' => t('Add user'),
     '#validate' => array('rawpheno_admin_validate_add_user'),
     '#submit' => array('rawpheno_admin_submit_add_user'),
     '#limit_validation_errors' => array(
       array('txt_autocomplete_user'),
       array('txt_id')
     ),
   );


  // Environement Data File Upload.
  $inline_wraper = array(
    '#prefix' => '<div class="envdata-field-wrapper">',
    '#suffix' => '</div>',
  );

  $form['fieldset_envdata'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload Environment Data File'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );

  // Add seach field to filter the list of name.
  $form['fieldset_envdata']['file'] = array(
    '#title' => 'File :',
    '#type' => 'file',
  ) + $inline_wraper;

  // Location: fetch all location in a project based on uploaded data.
  $sql = "SELECT location
          FROM {rawpheno_rawdata_mview} INNER JOIN pheno_plant_project USING(plant_id)
          WHERE project_id = :project_id
          GROUP BY location
          ORDER BY location ASC";

  $locations = chado_query($sql, $args);
  $location_options = ($locations->rowCount() > 0) ? $locations->fetchAllKeyed(0,0) : array();

  $form['fieldset_envdata']['select_location'] = array(
    '#title' => 'Location :',
    '#type' => 'select',
    '#options' => $location_options,
    '#empty_option' => t('- Select -'),

    '#ajax' => array(
      'wrapper' => 'envdata-year',
      'callback' => 'rawpheno_ajax_fetch_years',
      'progress' => array('type' => 'throbber', 'message' => ''),
    ),
  ) + $inline_wraper;

  // Year: fetch all years in a project given a location.
  if (isset($form_state['values']['select_location']) &&
      $form_state['values']['select_location'] != '') {

    $location_value = $form_state['values']['select_location'];

    $sql = "SELECT SUBSTRING(planting_date, 1, 4) AS year
            FROM {rawpheno_rawdata_mview}
            WHERE location = :location
              AND plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id = :project_id)
            ORDER BY year DESC";

    $years = chado_query($sql, $args + array(':location' => $location_value));
    // Assoc. array will remove any duplicates in the query result.
    $year_options = ($years->rowCount() > 0) ? $years->fetchAllKeyed(0,0) : array();
  }
  else {
    $year_options = array();
  }

  $inline_wraper['#prefix'] = '<div id="envdata-year" class="envdata-field-wrapper">';
  $form['fieldset_envdata']['select_year'] = array(
    '#title' => 'Year :',
    '#type' => 'select',
    '#options' => $year_options,
    '#empty_option' => t('- Select -'),
  ) + $inline_wraper;

  // Submit:
  $form['fieldset_envdata']['upload_env_file'] = array(
    '#type' => 'submit',
    '#value' => t('Upload file'),
    '#validate' => array('rawpheno_admin_validate_upload_envdata'),
    '#submit' => array('rawpheno_admin_submit_upload_envdata'),
    '#limit_validation_errors' => array(
      array('filefield_file'),
      array('select_location'),
      array('select_year'),
    ),
  );

  // DISPLAY SUMMARY OF DATA REQUIRED BY THIS INTERFACE.

  // List column headers in a project.
  // Given a project id, select all column headers belonging to that project.
  // Note the loding (scale 1-5) upright-lodged header is the only header using this
  // format (all headers use the format: name (trait rep; unit), thus a flag indicating
  // if header is such is required to disable edit option and prevent edit functionality
  // from processing unfamiliar format.
  $sql = "SELECT project_cvterm_id, pheno_project_cvterm.type
          FROM {cvterm} RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
          WHERE project_id = :project_id ORDER BY type, name ASC";

  $headers = chado_query($sql, $args);

  // List active users in a project.
  $sql = "SELECT project_user_id, uid, name, mail, created, status, login
          FROM {users} RIGHT JOIN {pheno_project_user} USING(uid)
          WHERE project_id = :project_id
          ORDER BY name DESC";

  $users = db_query($sql, $args);

  // Environment data files.
  $sql = "SELECT environment_data_id, location, year, sequence_no, filename, uri, timestamp
          FROM {pheno_environment_data} INNER JOIN file_managed USING(fid)
          WHERE project_id = :project_id
          ORDER BY location, year, sequence_no ASC";

  $envdata = db_query($sql, $args);

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
  $has_name = 0;
  $count_essential = 0;

  if ($headers->rowCount() > 0) {
    $i = 0;
    foreach($headers as $h) {
      if ($h->type == $trait_type['type1']) {
        $count_essential++;
      }

      // Get header information.
      $header_asset = rawpheno_function_header_properties($h->project_cvterm_id);

      // Edit link.
      $edit_cell = '-';
      if ($header_asset['name'] != 'Lodging (Scale: 1-5) upright - lodged') {
        $edit_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $h->project_cvterm_id . '/header/edit';
        $edit_cell = l('Edit', $edit_link);
      }

      // Delete link.
      $del_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $h->project_cvterm_id . '/header/delete';
      $del_cell = l('Remove', $del_link, array('attributes' => array('class' => 'link-del')));

      // No edit and delete when column header is of type plantproperty.
      if ($header_asset['type'] == $trait_type['type4'] OR $header_asset['name'] == 'Planting Date (date)') {
        // Add Name column header to the row array.
        if ($has_name == 0 AND $header_asset['name'] != 'Planting Date (date)') {
          array_push($arr_rows, array((count($arr_rows) + 1), '<h4>Name</h4>', 'Name', 'PLANTPROPERTY', '-', '-'));
          $has_name += 1;
          $i++;
        }

        $edit_cell = $del_cell = '-';
      }

      $class = ($header_asset['type'] == $trait_type['type1']) ? 'essential-trait' : 'non-essentialtrait';

      if ($header_asset['type'] == $trait_type['type4']) {
        // Row where trait is plantproperty.
        $header_cell = '<h4>' . $header_asset['name'] . '</h4>';
        $definition_cell = $header_asset['name'];
      }
      else {
        // All traits.
        $header_cell = '<h4>' . $header_asset['name'] . '</h4><p class="r-ver">' . rawpheno_admin_mark_empty($header_asset['r_version']) . '</p>';
        $definition_cell = '<p><strong>DEFINITION:</strong><br /> ' . @rawpheno_admin_mark_empty($header_asset['definition']) .  '</p>' .
                           '<p><strong>COLLECTION METHOD:</strong><br />' . @rawpheno_admin_mark_empty($header_asset['method']) . '</p>';
      }

      // Register a row.
      $arr_rows[] = array(
        ($i+1),           // Row counter.
        $header_cell,     // Trait/header name.
        $definition_cell, // Trait/header definition.
        array(
          'data' => strtoupper($header_asset['type']),
          'class' => array($class)
        ),                // Add class to cell to highlight text when trait is essential trait.
        $edit_cell,       // A link to edit a trait.
        $del_cell         // A link to delete a trait.
      );

      $i++;
    }
  }
  else {
    // When no records found, update the empty table property.
    $arr_tbl_args['empty'] = t('No column headers in this project');
  }

  // Warn admin that project has no essential trait
  if ($count_essential < 1) {
    $form['message_no_essential'] = array(
     '#markup' => '<div class="messages warning">Project has no Essential Column Header.</div>'
    );
  }

  // Warn admin that project has no assigned user.
  if ($users->rowCount() < 1) {
    $form['message_no_essential'] = array(
     '#markup' => '<div class="messages warning">Project has no active users.</div>'
    );
  }

  // Tabs to show either the project column headers table or project active users table.
  // Add 1 to account for Name column header.
  $form['nav_tabs'] = array(
    '#markup' => '<div id="nav-tabs">
                    <ul>
                      <li class="active-tab">' . ($headers->rowCount() + 1) . ' Column Headers</li>
                      <li>' . $users->rowCount() . ' Active Users</li>
                      <li>' . $envdata->rowCount() . ' Environment Data File</li>
                    </ul>
                  </div>'
  );


  // Table headers.
  array_push($arr_headers, '-', t('Column Header <small>(unit)</small>'), t('Definition/Collection Method'), t('Type'), t('Edit'), t('Remove'));

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
  // Construct table that lists all active users to a project along with files
  // associated to a user in a given project.
  // Options to delete items give admin record management functionality.
  // NOTE: Delete option will not physically delete a record in users table.
  //       When a user is deleted, it is removed from the project only.

  $arr_rows = $arr_headers = array();

  // Query user backup files.
  $sql_F = "
    SELECT t1.*, t2.filename, t2.filesize, t2.timestamp, t2.uri
    FROM pheno_backup_file AS t1 INNER JOIN file_managed AS t2 USING(fid)
    WHERE t1.project_user_id = :project_user_id ORDER BY t1.version DESC";


  // Table rows
  if ($users->rowCount() > 0) {
    $i = 0;
    foreach($users as $u) {
      if (empty($u->name)) {
        continue;
      }

      // Delete link.
      $del_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $u->project_user_id . '/user/delete';
      $del_cell = l('Remove', $del_link, array('attributes' => array('class' => 'link-del')));

      // Create a row of user with other relevant user info.
      $arr_rows[] = array(
        ($i+1),                                         // Row number.
        '<div class="tag-name">' . $u->name . '</div>', // Name of user.
        $u->mail,                                       // Email address.
        '<em>' . format_date($u->login) . '</em>',      // Date of last login.
        $del_cell                                       // Link to remove user from a project.
      );

      // Create user sub-table listing files per user account.
      $args_F = array(':project_user_id' => $u->project_user_id);
      $result_F = db_query($sql_F, $args_F);
      $my_files_count = $result_F->rowCount();


      $arr_header_F = array('File', 'Version', 'Archive', 'Notes', 'Validation Result');

      $arr_rows_F = array();
      if ($my_files_count > 0) {
        foreach($result_F as $f) {
          // File.
          $cell_file = l($f->filename . ' (' . format_size($f->filesize) . ')', file_create_url($f->uri),
            array('attributes' => array('target' => '_blank'))
          );

          $cell_file .= t('<br /><small>Uploaded: @timestamp</small>', array('@timestamp' => format_date($f->timestamp)));

          // Is archive?
          $cell_is_archive = ($f->archive == 'y') ? 'Yes' : 'No';

          // Notes.
          $cell_notes = (empty($f->notes))
            ? '&nbsp;'
            : '<div class="container-cell" title="Click to expand or collapse" id="vn-file-' . $f->file_id . '">' . $f->notes . '</div>';

          // Validation Result.
          $vr_tmp = str_replace(array('#item: (passed)', '#item: (failed)'),
                                array('<p class="pass"> (PASSED)', '<p class="fail"> * (FAILED)'),
                                $f->validation_result) . '</p>';
          $alert = '';
          if (($n = substr_count($vr_tmp, 'FAILED')) > 0) {
            $alert = '<span class="alert-error" title="File has failed validation notices...">' . $n . ' Validation errors</span>';
          }

          $cell_validation = $alert . '<div class="container-cell" id="vr-file-' . $f->file_id . '" title="Click to expand or collapse">'. $vr_tmp .'</div>';

          // Create row showing backed up file with file description.
          $arr_rows_F[] = array(
            $cell_file,        // Filename.
            '#' . $f->version, // File version.
            $cell_is_archive,  // Indicate if file is archived or not.
            $cell_notes,       // Notes, comments to file.
            $cell_validation   // Validation result performed to the file.
          );
        }
      }

      $file_table = theme('table',
        array('header' => $arr_header_F,
              'rows' => $arr_rows_F,
              'attributes' => array('id' => 'tbl-my-files'))
      );

      $link_show_folder = ($my_files_count > 0)
        ? l('[Show]', '#', array('attributes'=> array('id' => 'folder-' . $u->project_user_id, 'class'=>'link-show-folder')))
        : '';

      // Account status.
      $acc_status = ($u->status == 1) ? 'Active' : 'Suspended';

      $markup = sprintf('
        <span><strong>[ACCOUNT]</strong> Status: %s | Created: %s - %d Files uploaded %s</span>
        <div id="show-folder-'. $u->project_user_id .'" class="div-my-folder">
          <div class="container-my-folder">%s</div>
        </div>', $acc_status, format_date($u->created), $my_files_count, $link_show_folder, $file_table);

      // Create markup (container for table of files) to show user backed up files.
      $arr_rows[] = array(
        'data' => array(
          array(
            'data' => $markup,
            'colspan' => 5,
            'class' => 'row-user-my-folder',
          )
        )
      );

      $i++;
    }
  }
  else {
    // When no records found, update the empty table property.
    $arr_tbl_args['empty'] = t('No active user in this project');
  }

  // Table headers.
  array_push($arr_headers, '-', t('Name'), t('Email Address'), t('Last Login'), t('Remove'));

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

  //
  $arr_rows = $arr_headers = array();

  // Environment Data.
  if ($envdata->rowCount() > 0) {
    // Table headers.
    array_push($arr_headers, '-', t('File'), t('Location'), t('Year'), t('Sequence No.'), t('Delete'));

    // Query environment data files.
    $sql = "SELECT environment_data_id, filename, location, year, sequence_no, timestamp, uri, filesize
            FROM pheno_environment_data INNER JOIN file_managed USING(fid)
            WHERE project_id = :project_id
            ORDER BY location, year, sequence_no DESC";

    $env_datafile = chado_query($sql, $args);

    foreach($env_datafile as $i => $env) {
      $cell_file = l($env->filename . ' (' . format_size($env->filesize) . ')', file_create_url($env->uri),
        array('attributes' => array('target' => '_blank'))
      );

      $cell_file .= t('<br /><small>Uploaded: @timestamp</small>', array('@timestamp' => format_date($env->timestamp)));

      $del_link = 'admin/tripal/extension/rawphenotypes/all_projects/' . $env->environment_data_id . '/envdata/delete';
      $cell_del = l('Delete', $del_link, array('attributes' => array('class' => 'link-del')));

      $arr_rows[] = array($i+1, $cell_file, $env->location, $env->year, '#' . $env->sequence_no, $cell_del);
    }
  }
  else {
    // When no records found, update the empty table property.
    $arr_tbl_args['empty'] = t('No environment data file in this project.');
  }

  // Update table properties.
  $arr_tbl_args['header'] = $arr_headers;
  $arr_tbl_args['rows'] = $arr_rows;
  $arr_tbl_args['attributes']['id'] = 'tbl-project-envdata';

  // Render the table element.
  $form['tbl_project_envdata'] = array(
    '#markup' => theme('table', $arr_tbl_args),
    '#prefix' => '<div id="container-prj-env" class="container-table-data tab-data">',
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * AJAX Function callback.
 */
function rawpheno_ajax_fetch_years($form, &$form_state) {
  return $form['fieldset_envdata']['select_year'];
}

function ajax_rawpheno_filter_users($form, &$form_state) {
  return $form['fieldset_users']['tbl_users'];
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
  $breadcrumbs = array();
  $breadcrumbs[] = l('Home', '');
  $breadcrumbs[] = l('Rawphenotypes', '/admin/tripal/extension/rawphenotypes');
  $breadcrumbs[] = l('Manage Projects', '/admin/tripal/extension/rawphenotypes/all_projects');

  drupal_set_breadcrumb($breadcrumbs);

  // Set the title of this page. This line will create a page title on upper left hand
  // corner of the page preceding the tabs.
  drupal_set_title('Manage Project Assets / Update Header');

  if ($action == 'edit') {
    if ($header_asset['name'] == 'Lodging (Scale: 1-5) upright - lodged' OR $header_asset['name'] == 'Comments') {
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
        '#markup' => l('Go back to projects table', 'admin/tripal/extension/rawphenotypes/all_projects') . ' | ' . l(t('Go back to project column headers table') ,'admin/tripal/extension/rawphenotypes/all_projects/' . $header_asset['in_project_id'] . '/project/manage'),
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
      // Include the project id when modifying a trait header.
      $default_values = array('count_project'        => $header_asset['count_project'],
                              'count_data'           => $header_asset['count_data'],
                              'txt_id'               => $header_asset['cvterm_id'],
                              'prj_id'               => $header_asset['in_project_id'],
                              'txt_trait_name'       => trim($trait_name),
                              'sel_trait_rep'        => trim($trait_rep),
                              'sel_trait_unit'       => trim($trait_unit),
                              'txt_trait_definition' => $header_asset['definition'],
                              'txt_trait_method'     => $header_asset['method'],
                              'txt_trait_rfriendly'  => $header_asset['r_version'],
                              'sel_trait_type'       => $header_asset['type'],
                              'btn_trait_submit'     => 'Save');

      $form = rawpheno_admin_render_form_trait($form, $form_state, $default_values);

      // TABLE
      // Construct a table, as a summary, showing information about the header.

      $markup = '<h2>SUMMARY: ' . $header_asset['name'] . '</h2> PROJECT: ' . $header_asset['in_project_name'];

      $header_cell = $header_asset['name'] . '<p class="r-ver">' . rawpheno_admin_mark_empty($header_asset['r_version']) . '</p>';
      $definition_cell = '<p><strong>DEFINITION:</strong><br /> ' . @rawpheno_admin_mark_empty($header_asset['definition']) .  '</p>' .
                         '<p><strong>COLLECTION METHOD:</strong><br />' . @rawpheno_admin_mark_empty($header_asset['method']) . '</p>';


      // Array to hold table properties.
      $arr_tbl_args = array(
        'header' => array('-', t('Column Header <small>(unit)</small>'), t('Collection Method'), t('Definition'), t('Type')),
        'rows' => array(
          array(1,
                $header_cell,
                $header_asset['method'],
                $header_asset['definition'],
                strtoupper($header_asset['type'])
          )
        ),
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

    if (strpbrk($form_state['values']['txt_trait_name'], '()')) {
      // Test if user added parenthesis in the name.
      form_set_error('txt_trait_name', t('Characters "(" and/or ")" found in column header name. Please remove these characters and try again.'));
    }
    else {
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
}


/**
 * Implements hook_submit().
 * Add project assets (headers and users).
 */
function rawpheno_admin_project_management_submit($form, &$form_state) {
  // Get the submit button that triggered the submit action. Since this is a general hook_submit()
  // of the entire form, limit the proces only to submit button in from add column header and save column header.
  $btn_submit = $form_state['triggering_element']['#value'];

  if ($btn_submit == 'Add column header' OR $btn_submit == 'Save') {
    // Holds the record id number.
    $asset_id = $form_state['values']['txt_id'];

    // Get field values.
    $unit = $form_state['values']['sel_trait_unit'];

    // Construct the trait name based on given name, rep and unit.
    $trait_name = rawpheno_admin_construct_trait_name(
      array('name' => $form_state['values']['txt_trait_name'],
            'rep'  => $form_state['values']['sel_trait_rep'],
            'unit' => $form_state['values']['sel_trait_unit'])
    );

    // R Friendly version of the Header name.
    // When supplied, use it, otherwise transform the name to R friendly.
    $rver = empty($form_state['values']['txt_trait_rfriendly'])
      ? rawpheno_function_make_r_compatible($trait_name)
      : $form_state['values']['txt_trait_rfriendly'];

    $trait_def  = $form_state['values']['txt_trait_def'];
    $col_method = $form_state['values']['txt_trait_method'];
    $trait_type = $form_state['values']['sel_trait_type'];

    $cv_type = tripal_get_cv(array('name' => 'phenotype_measurement_types'));
    $cv_rver = tripal_get_cv(array('name' => 'phenotype_r_compatible_version'));
    $cv_desc = tripal_get_cv(array('name' => 'phenotype_collection_method'));
    $cv_unit = tripal_get_cv(array('name' => 'phenotype_measurement_units'));

    // Get the cvterm id of the unit selected.
    $cvterm_unit = tripal_get_cvterm(array('name' => $unit, 'cv_id' => $cv_unit->cv_id));


    // ADD.
    if ($btn_submit == 'Add column header') {
      // Uses project id.
      // Insert cvterm.
      $cvterm = tripal_insert_cvterm(
        array(
          'id' => 'rawpheno_tripal:' . $trait_name,
          'name' => $trait_name,
          'cv_name' => 'phenotype_measurement_types',
          'definition' => $trait_def
        )
      );

      // Add a cvterm prop to store the R friendly version.
      chado_insert_record('cvtermprop',
        array(
          'cvterm_id' => $cvterm->cvterm_id,
          'type_id' => $cv_rver->cv_id,
          'value' => $rver,
          'rank' => 0
        )
      );

      // Add a cvter prop to store the collection method.
      chado_insert_record('cvtermprop',
        array(
          'cvterm_id' => $cvterm->cvterm_id,
          'type_id' => $cv_desc->cv_id,
          'value' => $col_method,
          'rank' => 0
        )
      );

      // Relate the cvterm to unit.
      chado_insert_record('cvterm_relationship',
        array(
          'type_id' => $cv_unit->cv_id,
          'object_id' => $cvterm->cvterm_id,
          'subject_id' => $cvterm_unit->cvterm_id,
        )
      );

      // Add entry to project cvterms table.
      db_insert('pheno_project_cvterm')
        ->fields(
          array(
            'project_id' => $asset_id,
            'cvterm_id' => $cvterm->cvterm_id,
            'type' => $trait_type
          )
        )
        ->execute();

      drupal_set_message(t('You have successfully added trait to this project.'), 'status');
    }
    // UPDATE.
    else {
      // Project id the trait is in.
      $prj_id = $form_state['values']['prj_id'];

      // Uses cvterm id number.
      // Update cvterm record (name and definition).
      chado_update_record('cvterm',
        array(
          'cvterm_id' => $asset_id,
          'cv_id' => $cv_type->cv_id
        ),
        array(
          'name' => $trait_name,
          'definition' => $trait_def
        )
      );

      // Update cvtermprop (rfriendly version).
      chado_update_record('cvtermprop',
        array(
          'cvterm_id' => $asset_id,
          'type_id' => $cv_rver->cv_id
        ),
        array(
          'value' => $rver
        )
      );

      // Update method of collection.
      // When updating, make sure term has a collection method entry in cvtermprop.
      // If none, add an entry.
      $sql = "SELECT cvterm_id FROM {cvtermprop}
              WHERE
                cvterm_id = :cvterm_id
                AND type_id = :cv_method
              LIMIT 1";

      $args = array(':cvterm_id' => $asset_id, ':cv_method' => $cv_desc->cv_id);
      $result = chado_query($sql, $args);

      if ($result->rowCount() == 1) {
        chado_update_record('cvtermprop',
          array(
            'cvterm_id' => $asset_id,
            'type_id' => $cv_desc->cv_id
          ),
          array(
            'value' => $col_method
          )
        );
      }
      else {
        chado_insert_record('cvtermprop',
          array(
            'cvterm_id' => $asset_id,
            'type_id' => $cv_desc->cv_id,
            'value' => $col_method,
            'rank' => 0
          )
        );
      }

      // Update cvterm relationship (cvterm - unit).
      chado_update_record('cvterm_relationship',
        array(
          'object_id' => $asset_id
        ),
        array(
          'subject_id' => $cvterm_unit->cvterm_id
        )
      );

      // Update trait type.
      db_update('pheno_project_cvterm')
        ->fields(array('type' => $trait_type))
        ->condition('cvterm_id', $asset_id, '=')
        ->condition('project_id', $prj_id, '=')
        ->execute();

      drupal_set_message(t('You have successfully updated a column header in this project.'), 'status');
    }
  }
}


/**
 * Function callback: Delete a user from a project.
 * Note: The record of user is not physically deleted from the database.
 * When deleting a user, it is only removed from the assigned project.
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
 * Function callback: Delete environment data file from a project.
 *
 * @param $envdata_project_asset
 *   An array containing data and file information about an environment data file.
 */
function rawpheno_admin_project_envdata($form, $form_state, $envdata_project_asset, $action) {
  if ($action == 'delete') {
    db_delete('pheno_environment_data')
      ->condition('environment_data_id', $envdata_project_asset['envdata_id'])
      ->execute();

    // To delete a file in Drupal use file_delete function.
    // Get file object and load it to file delete funciton.
    $file = file_load($envdata_project_asset['fid']);
    file_delete($file, TRUE);

    // Assign a new sequence. This will maintain the order of sequence number.
    $sql = "SELECT * FROM pheno_environment_data
            WHERE project_id = :project_id AND location = :location AND year = :year
            ORDER BY sequence_no ASC";

    $result = chado_query($sql, array(
      ':project_id' => $envdata_project_asset['project_id'],
      ':location' => $envdata_project_asset['location'],
      ':year' => $envdata_project_asset['year'],
    ));

    if ($result->rowCount() > 0) {
      foreach($result as $seq => $row) {
        // Record.
        chado_query("UPDATE pheno_environment_data SET sequence_no = :new_seqno WHERE environment_data_id = :envdata_id",
        array(
          ':new_seqno' => $seq + 1,
          ':envdata_id' => $row->environment_data_id,
        ));
      }
    }

    // Redirect user.
    $path = url('../../admin/tripal/extension/rawphenotypes/all_projects/' . $envdata_project_asset['project_id'] . '/project/manage');
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
  // Allow only to add essential, optional, contributed or subset column header types.
  unset($a['type4']);

  // Type is no user contributed. Do not suggest contributed so user will have not option to add contributed
  // trait using admin since contributed traits are traits derived from submitted spreadsheet in stage 01 : Describe New Trait.
  if (!isset($default['sel_trait_type']) || $default['sel_trait_type'] != $a['type5']) {
    unset($a['type5']);
  }

  // Get the values and use it as both key and value.
  $t = array_values($a);
  $trait_type = array_combine($t, array_map('strtoupper', $t));

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

  // Project id the trait is in or the trait id number.
  $form['fieldset_trait']['txt_id'] = array(
    '#type' => 'hidden',
    '#value' => $default['txt_id'],
  );

  // Exclusive to modifying trait, include the project id the trait is registered.
  if (isset($default['prj_id']) && $default['prj_id'] > 0) {
    $form['fieldset_trait']['prj_id'] = array(
      '#type' => 'hidden',
      '#value' => $default['prj_id'],
    );
  }

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
    $reps_val[] = $r . ';';
  }
  $trait_rep = array('' => 'None') + array_combine($reps_val, $reps);

  $form['fieldset_trait']['sel_trait_rep'] = array(
    '#type' => 'select',
    '#title' => t('Trait Rep/Stages:'),
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
    '#options' => $unit,
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
    '#disabled' => FALSE,
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
    '#type' => 'textarea',
    '#title' => t('Definition:'),
    '#description' => t('A human-readable text definition'),
    '#required' => TRUE,
    '#default_value' => isset($default['txt_trait_definition']) ? $default['txt_trait_definition'] : '',
    '#disabled' => $disabled,
  );

  // Describe method of collection field.
  $form['fieldset_trait']['txt_trait_method'] = array(
    '#type' => 'textarea',
    '#title' => t('Describe Method:'),
    '#description' => t('Describe the method used to collect this data if you used a scale, be specific'),
    '#required' => TRUE,
    '#default_value' => isset($default['txt_trait_method']) ? $default['txt_trait_method'] : '',
    '#disabled' => $disabled,
  );

  // Tell user about contributed trait when detected. Such trait is not included when generating data collection
  // spreadsheet file - to incorporate it, please set the trait type to either essential or optional.
  if (isset($a['type5']) && isset($default['sel_trait_type']) && $default['sel_trait_type'] == $a['type5']) {
    // The header is contributed. Indicate to user as such.
    $form['fieldset_trait']['warning_contributed'] = array(
      '#markup' => '<div class="messages warning">This column header is a user contributed trait and is not incorporated in generating Data Collection Spreadsheet file for this Project. To include this trait, set the trait type to Essential or Optional.</div>',
    );
  }

  // Trait is essential field. Default to unchecked.
  $form['fieldset_trait']['sel_trait_type'] = array(
    '#type' => 'select',
    '#title' => t('Type:'),
    '#description' => t('Set the type to <span class="essential-trait">ESSENTIAL</span> to ensure this header must exists in the spreadsheet file'),
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
  // Uses project id.
  $project_id = $form_state['values']['txt_id'];
  // All Types.
  $types = rawpheno_function_trait_types();

  // The type of header is set to OPTIONAL trait.
  foreach(array_filter($form_state['values']['tbl_existing_headers']) as $m) {
    if (isset($form_state['input']['traittype-' . $m]) AND $form_state['input']['traittype-' . $m] == 1) {
      $trait_type = $types['type1'];
    }
    else {
      $trait_type = $types['type2'];
    }

    db_insert('pheno_project_cvterm')
      ->fields(
        array(
          'project_id' => $project_id,
          'cvterm_id' => $m,
          'type' => $trait_type)
      )
      ->execute();
  }

  drupal_set_message(t('You have successfully added column headers to this project.'), 'status');
}


/**
 * Function callback: Validate add existing column headers.
 */
function rawpheno_admin_validate_add_existing($form, &$form_state) {
  if (count(array_filter($form_state['values']['tbl_existing_headers'])) <= 0) {
    form_set_error('tbl_existing_headers', t('No column headers selected.'));
  }
}


/**
 * Function callback: Save values in add user form.
 */
function rawpheno_admin_submit_add_user($form, $form_state) {
  // Uses project id
  $project_id = $form_state['values']['txt_id'];
  $user = $form_state['values']['txt_autocomplete_user'];

  // Search user
  $sql = "SELECT uid FROM {users}
          WHERE
            name = :name
            AND uid NOT IN (SELECT uid FROM {pheno_project_user} WHERE project_id = :project_id AND status = 1)
          LIMIT 1";

  $args = array(':name' => $user, ':project_id' => $project_id);
  $u = db_query($sql, $args)
    ->fetchField();

  db_insert('pheno_project_user')
    ->fields(array(
      'project_id' => $project_id,
      'uid' => $u))
    ->execute();

  drupal_set_message(t('You have successfully added users to this project.'), 'status');
}


/**
 * Function callback: Validate add user.
 */
function rawpheno_admin_validate_add_user($form, &$form_state) {
  $project_id = $form_state['values']['txt_id'];
  $user = $form_state['values']['txt_autocomplete_user'];

  if (empty($user)) {
    form_set_error('txt_autocomplete_user', t('No user name supplied in the field.'));
  }
  else {
    // Search user
    $sql = "SELECT uid FROM {users} WHERE name = :name LIMIT 1";
    $args = array(':name' => $user);
    $u = db_query($sql, $args);

    if ($u->rowCount() < 1) {
      form_set_error('txt_autocomplete_user', t('User does not exist.'));
    }
    else {
      $user_id = $u->fetchObject();

      // Test if user was added twice in the same project.
      $sql = "SELECT uid FROM pheno_project_user WHERE project_id = :project_id AND uid = :user_id";
      $args = array(':project_id' => $project_id, ':user_id' => $user_id->uid);
      $u = db_query($sql, $args);

      if ($u->rowCount() > 0) {
        form_set_error('txt_autocomplete_user', t('User was already added and cannot be added again'));
      }
    }
  }
}

/**
 * Function callback: Save environment data file.
 */
function rawpheno_admin_submit_upload_envdata($form, &$form_state) {
  $project_id = $form_state['input']['txt_id'];
  $location   = $form_state['values']['select_location'];
  $year       = $form_state['values']['select_year'];

  if ($project_id && $location && $year) {
    $dir_public = 'public://';

    // File extension validator.
    $file_validate_ext = array(
      'file_validate_extensions' => array('xlsx csv tsv txt docx pptx'),
    );

    // Save env data file.
    $env_file = file_save_upload('file', $file_validate_ext, $dir_public, FILE_EXISTS_RENAME);

    if ($env_file) {
      // Rename file to include location, year, sequence no.
      // Fetch the largest seq no in a project, location and year.
      $sql = "SELECT sequence_no FROM pheno_environment_data
              WHERE location = :location AND year = :year AND project_id = :project_id
              ORDER BY sequence_no DESC LIMIT 1";

      $result = chado_query($sql, array(
        ':project_id' => $project_id,
        ':location' => $location,
        ':year'   => $year,
      ));

      $seq_no = ($result->rowCount() > 0) ? (int)$result->fetchField() + 1 : 1;
      $u_no = date('ymdis');

      $env_file_ext = pathinfo($env_file->filename, PATHINFO_EXTENSION);
      $new_filename = str_replace(array(' ', '-', ','), '_', $location) . '_' . $year . '_' . $u_no . '_environment_data.' . $env_file_ext;
      $new_filename = strtolower($new_filename);

      // Rename file.
      file_move($env_file, $dir_public . $new_filename);

      // Update file table, Make file permanent.
      $env_file->uri = $dir_public . $new_filename;
      $env_file->status = FILE_STATUS_PERMANENT;
      $env_file->filename = $new_filename;

      // Update file properties.
      file_save($env_file);
      // Tell Drupal, rawpheno is using this file.
      file_usage_add($env_file, 'rawpheno', 'file', $env_file->fid);

      // Create a row in the table.
      db_insert('pheno_environment_data')
        ->fields(array(
          'project_id' => $project_id,
          'fid'       => $env_file->fid,
          'location' => $location,
          'year'    => $year,
          'sequence_no' => $seq_no))
        ->execute();

      drupal_set_message(t('You have successfully uploaded environment data file to this project.'), 'status');
    }
  }
}

/**
 * Function callback: Validate environment data file.
 */
function rawpheno_admin_validate_upload_envdata($form, &$form_state) {
  // Location:
  if (empty($form_state['values']['select_location'])) {
    form_set_error('select_location', t('Location field is empty. Please select an option and try again.'));
  }

  // Year:
  if (empty($form_state['values']['select_year'])) {
    form_set_error('select_year', t('Year field is empty. Please select an option and try again.'));
  }

  // Environment Data File:
  $field_value = $_FILES['files']['name']['file'];

  if (empty($field_value)) {
    form_set_error('file', t('Environment Data File is empty. Please select a file and try again.'));
  }

  // File extension is handled by file_validate_extensions above.
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

  // Given the project id this user is assigned, test if it has any backup files.
  $sql = "SELECT COUNT(file_id) FROM {pheno_backup_file} WHERE project_user_id = :project_user_id";
  $file_count = db_query($sql, $args)
    ->fetchField();

  $arr_project_assets['project_file_count'] = $file_count;

  // Given the project id this user is assigned, test if it has data.
  $sql = "SELECT COUNT(plant_id) FROM {pheno_plant_project}
          WHERE project_id = (SELECT project_id FROM {pheno_project_user} WHERE project_id = :project_id AND uid = :user_id)";

  $args = array(':project_id' => $user->project_id, ':user_id' => $user->uid);
  $data_count = db_query($sql, $args)
    ->fetchField();

  $arr_project_assets['project_data_count'] = $data_count;

  return $arr_project_assets;
}

/**
 * Function fetch environment data properties.
 *
 * @param $asset_id
 *   An integer containing the environment data id number.
 *
 * @return
 *   An array containing data and file information of environment data.
 */
function rawpheno_admin_envdata_asset($asset_id) {
  $sql = "SELECT environment_data_id, project_id, location, year, fid
          FROM pheno_environment_data WHERE environment_data_id = :envdata_id
          LIMIT 1";

  $envdata = chado_query($sql, array(':envdata_id' => $asset_id));

  if ($envdata) {
    $e = $envdata->fetchObject();

    return array(
      'envdata_id' => $e->environment_data_id,
      'project_id' => $e->project_id,
      'location' => $e->location,
      'year' => $e->year,
      'fid' => $e->fid,
    );
  }
  else {
    return 0;
  }
}


function rawpheno_admin_mark_empty($val) {
  return (empty($val)) ? '-' : $val;
}
