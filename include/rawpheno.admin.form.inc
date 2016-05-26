<?php
/**
 * @file
 * Contains administrative interfaces for this module.
 */
 
/**
 * Function callback: construct administrative interface to modify page title and colour scheme.
 */
function rawpheno_admin_page_interface($form, &$form_state) {
  $form['rawpheno_colour_scheme'] = array(
    '#type' => 'textfield',
    '#title' => t('Enter colour:'),
    '#default_value' => variable_get('rawpheno_colour_scheme'),
    '#size' => 40,
    '#maxlength' => 20,
    '#description' => t('eg. HEX: #304356, blue | <a href="@adobe-kuler" target="_blank">get more colours here</a>', 
                        array('@adobe-kuler' => url('https://color.adobe.com/'))),
    '#required' => TRUE,
  );
  
  // Headers and title
  $form['fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Page title'),
    '#collapsed' => FALSE,
  );
    
    // Rawdata page.
    $form['fieldset']['rawpheno_rawdata_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Rawdata page:'),
      '#default_value' => variable_get('rawpheno_rawdata_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );
    
    // Download Page.
    $form['fieldset']['rawpheno_download_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Download page:'),
      '#default_value' => variable_get('rawpheno_download_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
    
    // Instructions page.
    $form['fieldset']['rawpheno_instructions_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Instructions page:'),
      '#default_value' => variable_get('rawpheno_instructions_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
  
    // Upload page.
    $form['fieldset']['rawpheno_upload_title'] = array(
      '#type' => 'textfield',
      '#title' => t('Title of Upload page:'),
      '#default_value' => variable_get('rawpheno_upload_title'),
      '#size' => 120,
      '#maxlength' => 220,
      '#required' => TRUE,
    );  
  
  return system_settings_form($form);
}

// Include function to manage column headers, cv terms and variable names.
module_load_include('inc', 'rawpheno', 'include/rawpheno.function.measurements');

/**
 * Function callback: construct administrative interface to manage R Friendly transformation of column headers.
 */
function rawpheno_admin_headers_interface($form, &$form_state) {
  // Fieldset to contain R-Friendly transformation rules form.
  $form['fieldset_rules'] = array(
    '#type' => 'fieldset',
    '#title' => t('R-Friendly Transformation Rules'),
    '#collapsed' => FALSE,
    '#collapsible' => FALSE,
  ); 
  
    $form['fieldset_rules']['rawpheno_admin_transform_words'] = array(
      '#type' => 'textarea',
      '#title' => t('List of words to remove:'),
      '#default_value' => variable_get('rawpheno_admin_transform_words'),
      '#description' => 'Separate words with commas',
    );

    $form['fieldset_rules']['rawpheno_admin_transform_characters'] = array(
      '#type' => 'textarea',
      '#title' => t('List of special characters to remove:'),
      '#default_value' => variable_get('rawpheno_admin_transform_characters'),
      '#description' => 'Separate characters with commas',
    );  
  
    $form['fieldset_rules']['rawpheno_admin_transform_replace'] = array(
      '#type' => 'textarea',
      '#title' => t('Match and replace (Match Character = Replacement):'),
      '#default_value' => variable_get('rawpheno_admin_transform_replace'),
      '#description' => 'Separate match and replace pairs with commas',
    );
    
    $form['fieldset_rules']['save_rules'] = array(
      '#type' => 'submit',
      '#value' => 'Save Rules',
    );
    
  return $form;
}

/**
 * Implements hook_submit().
 * Save R Friendly transformation rules and generated terms.
 */
function rawpheno_admin_headers_interface_submit($form, &$form_state) {
  // Save transformation rules.
  $words = $form_state['values']['rawpheno_admin_transform_words'];  
  variable_set('rawpheno_admin_transform_words', $words);  

  $chars = $form_state['values']['rawpheno_admin_transform_characters'];  
  variable_set('rawpheno_admin_transform_characters', $chars);  

  $replace = $form_state['values']['rawpheno_admin_transform_replace'];
  variable_set('rawpheno_admin_transform_replace', $replace);  
}

/**
 * Function callback: construct form to list all active projects.
 */
function rawpheno_admin_all_projects() {
  // Get trait types array.
  $trait_type = rawpheno_function_trait_types();
   
  // The select a project selection box contains projects without column headers assigned to it.
  // When selected, this will allow admin to manage (add and edit) column headers to a project.
  $sql = "SELECT t1.project_id, t1.name 
    FROM chado.project AS t1 
      LEFT JOIN pheno_project_cvterms AS t2 USING(project_id)
    WHERE t2.project_id IS NULL  
    ORDER BY t1.project_id DESC";
  
  $options = chado_query($sql)
    ->fetchAllKeyed();
    
  $form['fieldset_sel_project'] = array(
    '#type' => 'fieldset',
    '#title' => t('Manage Project Column Headers'),
  );
  
  $form['fieldset_sel_project']['sel_project'] = array(
    '#type' => 'select',
    '#options' => array('0' => 'Select Project') + $options,
    '#default_value' => reset($options),
  ); 
  
  // Make the select box into a jump menu.
  drupal_add_js('jQuery(document).ready(function() {
    jQuery("#edit-sel-project").change(function(){
      window.location.href = "./all_projects/project/"+ jQuery(this).val();
    });
  })', 'inline');
  
  // On the bottom of the select field, list the active projects or projects with 
  // column headers assigned to it.
  $sql = "SELECT t1.project_id, t1.name, COUNT(t2.cvterm_id) AS prj_trait_count, 
      COUNT(CASE WHEN t2.type = :essential_trait THEN 1 END) AS prj_essential_count
    FROM chado.project AS t1 LEFT JOIN pheno_project_cvterms AS t2 USING(project_id) 
    GROUP BY t1.project_id 
    HAVING COUNT(t2.cvterm_id) > 0
    ORDER BY prj_trait_count DESC";
  
  // Count traits of type essential.
  $project = chado_query($sql, array(':essential_trait' => $trait_type['type1']));
  
  $markup = '<h2>' . $project->rowCount() . ' Project(s) with column headers</h2>'; 
  $markup .= '<table class="tbl-project-traits sticky-enabled table-select-processed tableheader-processed sticky-table">
    <thead>
      <tr>
        <th width="10px" align="center">-</th>
        <th>Project Name</th> 
        <th width="20px">Column<br />Headers</th>
        <th width="20px">Essential<br />Headers</th>
        <th width="20px">View</th>
      </tr>
    </thead>
    <tbody>';

  $i = 0;
  foreach($project as $prj) {
    $class = ($i % 2) ? 'class="td-highlight"' : '';
    
    $markup .= '
      <tr ' . $class . '>
        <td>' . ($i + 1) . '</td>
        <td>' . $prj->name . '</td>
        <td>' . $prj->prj_trait_count . '</td>
        <td>' . $prj->prj_essential_count . '</td>
        <td><a href="./all_projects/project/' . $prj->project_id . '">View</a></td>
      </tr>';
    
    $i++;
  }  
  
  $markup .= '</tbody></table>';

  $form['all_projects'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );
    
  // Attach css and javascript.  
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.style.css'); 

  return $form;
}

/**
 * Function callback: construct form to list and add traits.
 *
 * @param $project_id
 *   An integer containing the project id number.  
 */
function rawpheno_admin_project($form, &$form_state = NULL, $project_id = NULL) {
  // Get trait types array.
  $trait_type = rawpheno_function_trait_types();
  
  // Link to go back to project table.
  $markup = '<p><a href="../../all_projects">Return to table of projects</a><br /><br /></p>';
  
  // Get project details.
  $project = chado_generate_var('project', array('project_id' => $project_id));
  $markup .= '<h2>PROJECT: ' . $project->name . '</h2>';

  $form['all_project_info'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );
  unset($markup);
  
  // FIEDLSET: Add/Edit Traits.
  // Allow administrator to add or edit traits to the current/active project.  
  $form['fieldset_trait'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add column header to this Project:'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  
  // Add form elements to add a new trait to a project.
  $default_values = array('txt_id' => $project_id, 'btn_trait_submit' => 'Add Trait');
  $form = rawpheno_admin_render_form_trait($form, $form_state, $default_values);
  
  $form['add_existing'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add existing column headers to this project:'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  
  $form['add_existing']['tip'] = array(
    '#markup' => '<em>Check the trait that you want to use in this project</em>'
  );
  
  // Query traits available in a project and list them in a table.
  $sql = "SELECT t1.type, t3.name AS cvterm,
      t4.value AS rfriendly, t3.cvterm_id, t1.project_id
    FROM pheno_project_cvterms AS t1 
      INNER JOIN chado.project AS t2 USING (project_id)
      INNER JOIN chado.cvterm AS t3 USING (cvterm_id) 
      INNER JOIN chado.cvtermprop AS t4 USING (cvterm_id) 
    WHERE t1.project_id = :project_id
    ORDER BY t1.type ASC";
    
  $args = array(':project_id' => $project_id);
  $result = chado_query($sql, $args);
  
  $markup = '<table class="tbl-project-traits sticky-enabled table-select-processed tableheader-processed sticky-table">
    <thead>
      <tr>
        <th width="10px">-</th>
        <th>Column Header <small>(unit)</small></th> 
        <th>R Friendly</th>
        <th width="20px">Type</th>
        <th width="20px">Edit</th>
        <th width="20px">Delete</th>
      </tr>
    </thead>
    <tbody>';

  $arr_project_traits = array();
  
  if ($result->rowCount() > 0) {
    $i = 0;
    foreach($result as $term) {
      array_push($arr_project_traits, $term->cvterm_id);
      $class = ($i % 2) ? 'class="td-highlight"' : '';
      
      $markup .= '
        <tr ' . $class . '>
          <td>' . ($i+1) . '</td>
          <td>' . $term->cvterm . '</td>
          <td>' . $term->rfriendly . '</td>
          <td>' . $term->type . '</td>';
      
      if ($term->type != $trait_type['type4']) {
        // Do not allow edit to column headers of type plantproperty.
        $markup .= '<td><a href="./' . $term->project_id . '/trait/' . $term->cvterm_id . '">Edit</a></td>
                    <td><a class="link-del" href="./' . $term->project_id . '/trait/' . $term->cvterm_id . '/del">Delete</a></td>';
      }
      else {
        $markup .= '<td>&nbsp;</td> <td>&nbsp;</td>';
      }
      
      $markup .= '</tr>';
      
      $i++;
    }
  }
  else {
    $markup .= '<tr><td colspan="6"><strong>No column headers available in this project.</strong></td></tr>';
  }  
  
  $markup .= '</tbody></table>'; 
  $markup .= '<a href="#" style="float:right">Go to top</a>';

  $form['all_project_traits'] = array(
    '#type' => 'markup',
    '#markup' => $markup,
  );
  
  // Confirm user they want to remove a trait from a project.
  drupal_add_js('jQuery(document).ready(function() {
    jQuery(".link-del").click(function(){
      var m = confirm("Are you sure you want to delete this trait from this project?");
      if(!m) return false;
    });
  })', 'inline');
  
  // Query all traits in cvterm that is not listed as among the traits in a project
  // and suggest it to admin.
  $sql = "SELECT t2.name, t2.cvterm_id, t2.definition
    FROM chado.cv AS t1 
      INNER JOIN chado.cvterm AS t2 USING(cv_id) 
    WHERE t1.name = 'phenotype_measurement_types'
    ORDER BY t2.cvterm_id ASC";
  
  $result = chado_query($sql);

  $header = array(
    'name' => t('Name'),
    'def' => t('Definition')
  );
  
  $options = array();
  foreach($result as $r) {
    if (!in_array($r->cvterm_id, $arr_project_traits)) {
      $options[$r->cvterm_id] = array(
        'name' => $r->name,
        'def' => $r->definition
      );
    }
  }
  
  $form['add_existing']['table'] = array(
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $options,
    '#js_select' => FALSE,
    '#attributes' => array('id' => 'tbl-existing-headers')
  );
  
  if (count($options) > 0) {
    $form['add_existing']['add_selected_trait'] = array(
      '#type' => 'submit',
      '#value' => t('Add Selected Traits'),
    );
  }
  
  // Attach css and javascript.  
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.style.css'); 
    
  return $form;
}

/**
 * Implements hook_form_validate() to check if trait name exists or is empty.
 */
function rawpheno_admin_project_validate($form, &$form_state) {
  $submit_btn = $form_state['triggering_element']['#value'];
  if ($submit_btn == 'Add Trait') {
    if (empty($form_state['values']['txt_trait_name'])) {
      // Check 1.
      // Trait name must have a value.
      form_set_error('txt_trait_name', t('Trait name must have a value'));
    }     
    else {
      // Check 2.
      // Trait should not match any traits in cvterm.

      // Trait name which contains name, rep and unit to be reconstructed
      // to cvterm format (trait name (trait rep; unit)
      $trait_name = rawpheno_admin_construct_trait_name(
        array('name' => $form_state['values']['txt_trait_name'], 
              'rep' => $form_state['values']['sel_trait_rep'],
              'unit'=> $form_state['values']['sel_trait_unit'])
      );      

      $t = tripal_get_cvterm(array('name' => $trait_name, 'cv_id' => array('name' => 'phenotype_measurement_types')));
      if (isset($t->cvterm_id)) {
        form_set_error('txt_trait_name', t('Trait name @trait_name already exists', array('@trait_name' => $trait_name)));
      }
    }    
  }
}

/**
 * Function callback: construct form to edit/modify a specific trait in a project.
 *
 * @param $cvtem_id
 *   An integer containing the unique id of a trait.
 */
function rawpheno_admin_project_trait($form, &$form_state = NULL, $prj_id = NULL, $cvterm_id = NULL, $do = NULL) {
  // Add a link to allow administrator to go back to the list of traits in a project.
  $form['project_trait_link'] = array(
    '#type' => 'markup',
    '#markup' => '<p><a href="../../' . $prj_id . '">Return to project column headers table</a><br /><br /></p>',
  );
  
  // To edit or delete, first test if the trait is a plant property type, if so, 
  // inform administrator that this action is not allowed on plant property type.
  // This check in case user will hack the url since traits of type plant property do
  // not have link  to edit and delete.
  $sql = "SELECT t1.id, t1.project_id, t2.name AS prj_name, t1.type, 
      t3.name AS cvterm, t4.value AS rfriendly, t3.cvterm_id, t3.definition
    FROM pheno_project_cvterms AS t1 
      INNER JOIN chado.project AS t2 USING (project_id)
      INNER JOIN chado.cvterm AS t3 USING (cvterm_id) 
      INNER JOIN chado.cvtermprop AS t4 USING (cvterm_id) 
    WHERE t1.cvterm_id = :cvterm_id AND t1.project_id = :prj_id
    LIMIT 1";
    
  $args = array(':cvterm_id' => (int)$cvterm_id, ':prj_id' => (int)$prj_id);
  $t = chado_query($sql, $args);
  
  if ($t->rowCount() == 1) {
    // Get trait types array.
    $trait_type = rawpheno_function_trait_types();
  
    // Trait is found.
    $trait = $t->fetchObject();  
    // Test to see if this trait is a plant property type.
    if ($trait->type == $trait_type['type4']) {
      // Trait is plant property type.
      drupal_set_message('Trait is a plant property type.');
    }
    else {
      // Trait is NOT plant property type.
      // Test if user wants to delete or edit.
      if (isset($do) AND $do == 'del') {
        // Admin wants to delete/remove the trait from the project.
        $del = "DELETE FROM {pheno_project_cvterms} WHERE id = :id";
        db_query($del, array(':id' => $trait->id));
        
        $path = url('../../admin/config/user-interface/rawpheno/all_projects/project/' . $prj_id);
        drupal_goto($path);
      }
      else {
        // Admin wants to edit/modify the trait.
        // FIEDLSET: Add/Edit Trait.
        // Contain all the form elements inside this fieldset.
        $form['fieldset_trait'] = array(
          '#type' => 'fieldset',
          '#title' => t('Eidt Trait:'),
          '#collapsed' => FALSE,
          '#collapsible' => FALSE,
        );
      
        // Trait name field.
        // Extract the trait rep value, trait unit and the trait name.
        $trait_rep = $trait_unit = '';
        // Extract the text value inside the parenthesis (unit part).
        $t = preg_match("/.*\(([^)]*)\)/", $trait->cvterm, $match);
        $u = (isset($match[1])) ? $match[1] : '';
   
        // Extract information in a unit only when there is a unit in the first place.
        // Get the Trait rep (eg R1, R7, 1st) and measurement unit (eg cm, days) values.
        if ($u != '') {
          // Split the information and see if either trait rep or unit is present.
          $p = explode(' ', $u);
          if (count($p) > 1) {
            // Has trait rep and unit.
            list($trait_rep, $trait_unit) = $p;
          }
          else {
            // Just the unit no trait rep.
            $trait_rep = '';
            $trait_unit = $u;
          }
        }      
      
        // When niether is present, use the defaul value of the trait rep and unit (null). 
        // Trait name without the unit part.
        $trait_name = preg_replace('/\(.*/', ' ', $trait->cvterm);
  
        // Add form elements to edit/modify a trait in a project.
        $default_values = array('txt_id' => $cvterm_id,
                                'txt_trait_name' => trim($trait_name),
                                'sel_trait_rep' => trim($trait_rep),
                                'sel_trait_unit' => trim($trait_unit),
                                'txt_trait_def' => $trait->definition,
                                'txt_trait_rfriendly' => $trait->rfriendly,
                                'sel_trait_type' => $trait->type,
                                'btn_trait_submit' => 'Save Trait');      
      
        $form = rawpheno_admin_render_form_trait($form, $form_state, $default_values);    
      
        // Display trait in a table as a summary.
        $markup = '<br /><h2>TRAIT SUMMARY: ' . $trait->cvterm . '</h2>';
        $markup .= '<table id="tbl-project-traits" class="sticky-enabled table-select-processed tableheader-processed sticky-table">
          <thead>
            <tr>
              <th>Trait <small>(unit)</small></th> 
              <th>R Friendly</th>
              <th>Definition</th>
              <th>Type</th>
            </tr>
          </thead>
          <tbody>';
        $markup .= '
          <tr>
            <td>' . $trait->cvterm  . '</td>
            <td>' . $trait->rfriendly . '</td>
            <td>' . $trait->definition . '</td>
            <td>' . $trait->type . '</td>
          </tr>';
        $markup .= '</tbody></table>'; 
        $markup .= '<em>PROJECT: ' . $trait->prj_name . '</em>';

        $form['project_trait_summary'] = array(
          '#type' => 'markup',
          '#markup' => $markup,
        );
  
        // Attach css and javascript.  
        $path = drupal_get_path('module', 'rawpheno') . '/theme/';
        $form['#attached']['css'] = array($path . 'css/rawpheno.style.css'); 
      }
    } 
  }
  else {
    // Trait does not exist.
    drupal_set_message('The trait does not exist.');
  }  

  return $form;
}

/**
 * Function callback: construct a form to either modify or add a trait.
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

  // Project id the trait is in.
  $form['fieldset_trait']['txt_id'] = array(
    '#type' => 'hidden',
    '#value' => $default['txt_id'],
  );
  
  // Trait name field.
  $form['fieldset_trait']['txt_trait_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Trait Name:*'),
    '#required' => FALSE,
    '#default_value' => isset($default['txt_trait_name']) ? $default['txt_trait_name'] : '',
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
  );  
  
  // Trait unit field. Default to none.
  // Query available units in chado cvterm of type phenotype_measurement_unit.
  $sql = "SELECT t2.name, t2.name || ' : ' || t2.definition
    FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING (cv_id) 
    WHERE t1.name = 'phenotype_measurement_units' 
    ORDER BY t2.name ASC";
    
  $unit = chado_query($sql)
    ->fetchAllKeyed();
  
  $form['fieldset_trait']['sel_trait_unit'] = array(
    '#type' => 'select',
    '#title' => t('Trait Unit:'),
    '#options' => array('' => 'None') + $unit,
    '#default_value' => isset($default['sel_trait_unit']) ? $default['sel_trait_unit'] : reset($unit),
  ); 
  
  // R Friendly field.
  $form['fieldset_trait']['txt_trait_rfriendly'] = array(
    '#type' => 'textfield',
    '#title' => t('R Friendly:'),
    '#description' => t('Leave this field blank to let system generate R Friendly version'),
    '#default_value' => isset($default['txt_trait_rfriendly']) ? $default['txt_trait_rfriendly'] : '',
  );   
  
  // Trait definition field.
  $form['fieldset_trait']['txt_trait_def'] = array(
    '#type' => 'textfield',
    '#title' => t('Trait Definition:'),
    '#required' => FALSE,
    '#default_value' => isset($default['txt_trait_def']) ? $default['txt_trait_def'] : '',
  );  
  
  // Trait is essential field. Default to unchecked.
  $form['fieldset_trait']['sel_trait_type'] = array(
    '#type' => 'select',
    '#title' => t('Type of trait:'),
    '#options' => $trait_type,
    '#default_value' => isset($default['sel_trait_type']) ? $default['sel_trait_type'] : reset($trait_type),
  );  

  // Save trait button.
  $form['fieldset_trait']['btn_trait_subtmit'] = array(
    '#type' => 'submit',
    '#value' => t('@save_or_add', array('@save_or_add' => $default['btn_trait_submit']))
  );
  
  return $form;      
}

/**
 * Implements a hook_submit() to save a trait into a project.
 */
function rawpheno_admin_project_submit($form, &$form_state) {
  $trait_type2 = rawpheno_function_trait_types();

  // Project id number a trait is to be saved.
  $id = $form_state['values']['txt_id'];
  
  $submit_btn = $form_state['triggering_element']['#value'];
  if ($submit_btn == 'Add Trait') {
    // Add trait using add trait form.  
    // Trait name which contains name, rep and unit to be reconstructed
    // to cvterm format (trait name (trait rep; unit)
    $trait_name = rawpheno_admin_construct_trait_name(
      array('name' => $form_state['values']['txt_trait_name'], 
            'rep' => $form_state['values']['sel_trait_rep'],
            'unit'=> $form_state['values']['sel_trait_unit'])
    );
  
    // Trait definition.
    $trait_def = $form_state['values']['txt_trait_def'];
    // Insert a cv term of type phenotype_measurement_types.
    $arr_terms_param = array('id' => 'tripal:' . $trait_name,
                             'name' => $trait_name,
                             'definition' => $trait_def,
                             'cv_name' => 'phenotype_measurement_types');
    
    $cvterm_obj = tripal_insert_cvterm($arr_terms_param);  
  
    // R Friendly version of the trait name.
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
    db_insert('pheno_project_cvterms')
      ->fields(array(
        'project_id' => $id,
        'cvterm_id' => $cvterm_obj->cvterm_id,
        'type' => $trait_type)
      )
      ->execute();
  }
  else {
    // Add trait/s using add existing trait form.
    foreach(array_filter($form_state['values']['table']) as $m) {
      db_insert('pheno_project_cvterms')
        ->fields(array(
          'project_id' => $id,
          'cvterm_id' => $m,
          'type' => $trait_type2['type1']))
        ->execute();
    }
  }
  
  // Test the project to see if it has all the plant property column headers.
  // Get the cvterm id of each plant property and test to see if they exists in a project.
  $sql = "SELECT cvterm_id FROM chado.cv AS t1 
      INNER JOIN chado.cvterm AS t2 USING(cv_id) 
    WHERE t1.name = 'phenotype_plant_property_types'
    ORDER BY cvterm_id ASC";
  
  $trait = chado_query($sql);
  
  // Test each plant property if it is present in the project.
  foreach($trait as $t) {
    $sql = "SELECT id FROM pheno_project_cvterms 
      WHERE cvterm_id = :cvterm_id AND project_id = :project_id
      LIMIT 1";
    
    $h = db_query($sql, array(':cvterm_id' => $t->cvterm_id, ':project_id' => $id));
    if ($h->rowCount() < 1) {
      // A particular plant property column header is not present in the project.
      // Add this column header to a project of type plantproperty.
      db_insert('pheno_project_cvterms')
      ->fields(array(
        'project_id' => $id,
        'cvterm_id' => $t->cvterm_id,
        'type' => $trait_type2['type4'])
      )
      ->execute();
    }
    else {
      // Found in the project, move to the next plant property.
      continue;
    }
  }
  
  // Tell user of the record insert.
  drupal_set_message('You have successfully added a new trait(s) into this project.');
}

/**
 * Implements a hook_submit() to save all modification to a trait.
 */
function rawpheno_admin_project_trait_submit($form, &$form_state) {
  // Edit Trait.
  // cvterm id number of the trait to be modified.
  $id = $form_state['values']['txt_id'];

  // Trait name which contains name, rep and unit to be reconstructed
  // to cvterm format (trait name (trait rep; unit)
  $trait_name = rawpheno_admin_construct_trait_name(
    array('name' => $form_state['values']['txt_trait_name'], 
          'rep' => $form_state['values']['sel_trait_rep'],
          'unit'=> $form_state['values']['sel_trait_unit'])
  );
  
  // Trait definition
  $trait_def = $form_state['values']['txt_trait_def'];
  
  // Update cvterm record (name and definition).
  $umatch = array('cvterm_id' => $id);
  $uvalues = array('name' => $trait_name, 'definition' => $trait_def);
  chado_update_record('cvterm', $umatch, $uvalues);      
  
  // Update cvtermprop (rfriendly version).
  $cv_rfriendly = tripal_get_cv(array('name' => 'r_compatible_column_header'));
  $rfriendly = (empty($form_state['values']['txt_trait_rfriendly'])) 
    ? rawpheno_function_make_r_compatible($trait_name)
    : $form_state['values']['txt_trait_rfriendly'];

  $umatch = array('cvterm_id' => $id, 'type_id' => $cv_rfriendly->cv_id);
  $uvalues = array('value' => $rfriendly);
  chado_update_record('cvtermprop', $umatch, $uvalues);      

  $trait_type = $form_state['values']['sel_trait_type'];
  
  db_update('pheno_project_cvterms')
    ->fields(array('type' => $trait_type))
    ->condition('cvterm_id', $id, '=')
    ->execute();
  
  // Tell user of the update.
  drupal_set_message('You have successfully updated a trait in this project.');
}

/**
 * Function to construct a trait given a name, trait rep and unit.
 *
 * @param $trait 
 *   An array containing name, trait and unit as its elements. 
 * 
 * @return 
 *   A string containing containing the trait in trait name (trait rep; unit) format.
 */
function rawpheno_admin_construct_trait_name($trait) {
  $unit = '';
  
  if (!empty($trait['unit'])) {
    $u = (empty($trait['rep'])) ? '' : $trait['rep'] . ' ';
    $unit = ' (' . $u . $trait['unit'] . ')';
  }  

  return ucfirst($trait['name'] . rtrim($unit));  
}