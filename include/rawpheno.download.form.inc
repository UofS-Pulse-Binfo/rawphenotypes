<?php
/**
 * @file
 * Contains download interface of this module.
 */

/**
 * Function callback: construct export data form.
 */
function rawpheno_download($form, &$form_state) {
  // Attach CSS and JavaScript
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.download.style.css');
  
  // Navigation button. Related page of download page is rawdata/summary page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('View Summary &#10095;'),
  );

  // PROJECT SELECT BOX.
  if (isset($form_state['values']['sel_project'])) {
    // Project selected.
    $project_selected = $form_state['values']['sel_project'];
  }
  
  $sql = "SELECT DISTINCT t1.project_id, t1.name
          FROM {project} AS t1 RIGHT JOIN pheno_plant_project AS t2 USING(project_id)
          ORDER BY t1.project_id DESC";
 
  $opt_project = chado_query($sql)
    ->fetchAllKeyed();
  
  if (count($opt_project) <= 0) {
    return $form;
  }
  
  // This is the error message used in lieu of Drupal form_set_error().
  // This error message will be in close proximity to the interface.
  $form['download_window_error'] = array(
    '#markup' => t('<div id="download-window-error" class="messages error">
                 Cannot generate CSV File. No trait or location selected.</div>'),
  );

  // AJAX wrapper.
  // Main wrapper
  $form['ajax_container'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="download-ajax-wrapper">',
    '#suffix' => '</div>',
  );

  // This a hidden field containing all project id.
  // This field will allow callback functions to get all project ids which is
  // the equivalent of the option select all project from the project select box.
  $form['ajax_container']['txt_project'] = array(
    '#type' => 'hidden',
    '#value' => implode(',', array_keys($opt_project)),
  );
  
  $form['ajax_container']['sel_project'] = array(
    '#type' => 'select',
    '#title' => t('Project'),
    '#options' => array(0 => 'All Projects') + $opt_project,
    '#multiple' => FALSE,
    '#id' => 'download-sel-project',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_get_locations_traits',
      'wrapper' => 'download-ajax-wrapper',
      'progress' => array('type' => '', 'message' => '')
    ),
  );
  
  // This will reset the project select box on load and page refresh.
  drupal_add_js('jQuery(document).ready(function() {
    jQuery("#download-sel-project").val(0);
  })', 'inline'); 
 
  // Define the project ids required by the next field.
  if (isset($project_selected)) {
    // When a project is selected.
    // 0 is the first option in the select box and translates to all projects.
    $project_id = ($project_selected == 0) ? array_keys($opt_project) : $project_selected;
  }
  else {
    // No project select. This is the defaul select all project.
    $project_id = array_keys($opt_project);
  }
  
  // LOCATION SELECT BOX.
  $sql = "SELECT DISTINCT value, value AS prj_location
          FROM pheno_plantprop 
          WHERE
            type_id = (SELECT cvterm_id FROM {cvterm} WHERE name = 'Location' LIMIT 1) AND
            plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id IN (:project_id))  
          ORDER BY value ASC";

  $opt_location = chado_query($sql, array(':project_id' => $project_id))
    ->fetchAllKeyed();
  
  $default_value = reset($opt_location);
  
  $form['ajax_container']['sel_location'] = array(
    '#type' => 'select',
    '#title' => t('Location'),
    '#options' => $opt_location,
    '#multiple' => TRUE,
    '#size' => 7,
    '#id' => 'download-sel-location',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_get_traits',
      'wrapper' => 'download-ajax-wrapper-traits',
      'progress' => array('type' => '', 'message' => '')
    ),
  );

  $form['ajax_container']['chk_select_all_locations'] = array(
    '#title' => t('Select all Locations'),
    '#type' => 'checkbox',
    '#default_value' => 0,
  );
  
  $location_id = $opt_location;

  // TRAITS.
  // Select traits wrapper.
  $form['ajax_container']['ajax_container_traits'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="download-ajax-wrapper-traits">',
    '#suffix' => '</div>',
  );
  
  // Get traits given a location and project.
  $opt_trait = rawpheno_download_load_traits($location_id, $project_id);
    
  $form['ajax_container']['ajax_container_traits']['sel_trait'] = array(
    '#type' => 'select',
    '#title' => t('@trait_count Traits available', array('@trait_count' => count($opt_trait))),
    '#options' => $opt_trait,
    '#multiple' => TRUE,
    '#size' => 15,
    '#id' => 'download-sel-trait',
  );
  
  $form['ajax_container']['ajax_container_traits']['chk_select_all_traits'] = array(
    '#title' => t('Select all Traits'),
    '#type' => 'checkbox',
    '#default_value' => 0,
  );

  $form['div_buttons'] = array(
    '#prefix' => '<div id="div-button">',
    '#suffix' => '</div>',
  );
  
  $form['div_buttons']['chk_rfriendly'] = array(
    '#title' => t('<img src="@img" alt="Make R Friendly" title="Make R Friendly" align="absmiddle" /> (Make R Friendly)', array('@img' => '../../' . $path . 'img/r.gif')),
    '#type' => 'checkbox',
    '#default_value' => 0,
  );

  $form['div_buttons']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
  );
  
  $form['#attached']['js']  = array($path . 'js/rawpheno.download.script.js');
    
  return $form;
}


/**
 * Function callback: AJAX update location and traits select boxes when project is selected.
 */
function rawpheno_download_get_locations_traits($form, $form_state) {
  return $form['ajax_container'];
}


/**
 * Function callback: AJAX update trait select box.
 */
function rawpheno_download_get_traits($form, $form_state) {
  $location = $form_state['values']['sel_location'];
  $project = $form_state['values']['sel_project'];
  
  // Determine if the selected project is all project.
  if ($project == 0) {
    // Yes, then read the value of the hidden field containing project ids.
    $t = $form_state['values']['txt_project'];
    $project = explode(',', $t);
  }
  
  // Get all traits given a location and project.
  $opt_trait = rawpheno_download_load_traits($location, $project);
  
  // Update the #options value of select a trait select box.
  $form['ajax_container']['ajax_container_traits']['sel_trait']['#options'] = $opt_trait;
  // Update the title.
  $form['ajax_container']['ajax_container_traits']['sel_trait']['#title'] = t('@count_trait Traits available', array('@count_trait' => count($opt_trait)));
  
  return $form['ajax_container']['ajax_container_traits'];
}


/**
 * Function get all traits given a location and project.
 * 
 * @param $location
 *   An array containing selected location from the location select box.
 * @param $project
 *   An array containing the selected project form the project select box.
 * @return
 *   An array containing all traits available.
 */
function rawpheno_download_load_traits($location, $project) {
 
 $sql = "SELECT DISTINCT t1.cvterm_id, t1.name 
         FROM 
           {cvterm} AS t1 
           RIGHT JOIN pheno_project_cvterm AS t2 USING(cvterm_id)
         WHERE 
           t2.project_id IN 
           
           (SELECT project_id 
            FROM pheno_plantprop INNER JOIN pheno_plant_project USING(plant_id)
            WHERE value IN (:location) AND project_id IN (:project))
           
           AND t1.name NOT IN ('Rep', 'Entry', 'Location', 'Name', 'Plot', 'Planting Date (date)', '# of Seeds Planted (count)')
        ORDER BY t1.cvterm_id ASC";

  $args = array(':location' => $location, ':project' => $project);
  $opt_trait = chado_query($sql, $args)
    ->fetchAllKeyed();
  
  return $opt_trait;
}


/**
 * Implements hook_form_submit().
 *
 * Generate a comma separated values (csv) file based on the location and trait set selected.
 */
function rawpheno_download_submit($form, &$form_state) {
  // Delay function not startle user with dialog box to save file.
  // A counter in front-end in sync whith this delay.
  sleep(4);
 
  // Project or projects selected by user
  $p = $form_state['values']['sel_project'];
  // Determine if user wanst all project, if so, use the text box containting
  // all project ids in the select project select box.
  if ($p == 0) {
    // Use the text box.
    $p = $form_state['values']['txt_project'];
    $project = explode(',', $p);
  }
  else {
    $project = $p;
  }
  // Location or locations selected by user.
  $location = $form_state['values']['sel_location'];
  // List of traits selected by user.
  $traits = $form_state['values']['sel_trait'];
  
  // Add planting date and # of seeds planted to the list of traits.
  // But, before doing this, make sure that these two column headers
  // are part or the project in the first place.
  $arr_notso_trait = array('Planting Date (date)', '# of Seeds Planted (count)');
  foreach($arr_notso_trait as $nt) {
    $sql = "SELECT cvterm_id FROM {cvterm} RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
            WHERE name = :cvterm AND project_id IN (:project_id) LIMIT 1";
    
    $n = chado_query($sql, array(':cvterm' => $nt, ':project_id' => $project));
    
    if ($n->rowCount() == 1) {
      array_push($traits, $n->fetchField());
    }
  }
  
  // Sub-query to select plant_id given a location and project.
  // NOTE: leading and trailing spaces are required.
  $sub_sql = " (SELECT plant_id 
                FROM {pheno_plantprop} INNER JOIN {pheno_plant_project} USING(plant_id)
                WHERE value IN (:location) AND project_id IN (:project)) ";

  // Query values required by sub query.
  $arr_q_string = array(':project' => $project, ':location' => $location, ':traits' => $traits);
    
  // First we need to get the header. This will allow us to ensure that the data
  // downloaded all matches up with the trait it is associated with. Furthermore,
  // it will allow us to handle missing data.
  $sql = "SELECT 'B' || t2.cvterm_id AS id, t2.name 
          FROM {pheno_plantprop} t1 LEFT JOIN chado.cvterm t2 ON t2.cvterm_id = t1.type_id 
          WHERE t1.plant_id IN" . $sub_sql . "
    
          UNION 
    
          SELECT 'C' || t2.cvterm_id AS id, t2.name 
          FROM {pheno_measurements} t1 LEFT JOIN chado.cvterm t2 ON t2.cvterm_id=t1.type_id 
          WHERE t2.cvterm_id IN (:traits) AND t1.plant_id IN" . $sub_sql . " 
          GROUP BY t2.cvterm_id, t2.definition";
  
  $result = db_query($sql, $arr_q_string)
    ->fetchAll();
  
  // Array to hold column headers. 
  //Add Name/Stock name column headers array.
  $header = array('A0' => 'Name');
  
  foreach ($result as $r) {
    $def = $r->name;
    
    // Get the R Friendly version when user requests for it.
    if ($form_state['values']['chk_rfriendly'] == 1) {
      // Get the R compatible version.
      $id_no = (int)trim(str_replace(array('B', 'C'), '', $r->id));
      $rfreindly = rawpheno_download_r_compatible($id_no);
      
      // When no equivalent R version is available, load the definition instead.
      $def = ($rfreindly == null) ? $r->name : $rfreindly;
    }
    
    // Column headers array.
    $header[ $r->id ] = $def;
  }
  
  // Sort array by key.
  ksort($header);
  
  // Query to join data from different tables.
  // Result: plant_id, trait_id, definition, data, and a grouping string
  // The result is sorted by plant_id and the grouping string ensuring that the first
  // row is Name - containing the stock name.
  // The result will be sorted into standard order: plot,entry,name,rep,location,traits.....
  
  // Thus first we select the name. Note that  the tid is 0 because this doesn't have a cvterm (ie: not a trait).
  $sql = "SELECT t2.plant_id AS id, '0' AS tid, 'Name' AS def, t1.name AS value, 'A' AS grp
          FROM {chado.stock} AS t1 INNER JOIN {pheno_plant} AS t2 USING(stock_id)
          WHERE t2.plant_id IN" . $sub_sql
      
          // Then we add on the required information (ie: plot, entry, rep, location)
          . "UNION 
    
          SELECT t1.plant_id AS id, t1.type_id AS tid, t2.name AS def, t1.value AS value, 'B' AS grp
          FROM {pheno_plantprop} AS t1
          INNER JOIN chado.cvterm AS t2 ON t1.type_id = t2.cvterm_id
          WHERE t1.plant_id IN" . $sub_sql
      
          // Finally we add in all the traits.
          . "UNION 
    
          SELECT t1.plant_id AS id, t1.type_id as tid, t2.name AS def, t1.value AS value, 'C' as grp
          FROM {pheno_measurements} AS t1
          INNER JOIN chado.cvterm AS t2 ON t1.type_id = t2.cvterm_id
          WHERE t2.cvterm_id IN (:traits) AND t1.plant_id IN" . $sub_sql
    
          // Lastly we order the results by plant_id and grouping string, and tid.
          . "ORDER BY id, grp, tid ASC";
  
  $results = db_query($sql, $arr_q_string);

  if ($results) {

    // Build a multi-dimensional array with all the data in it. This array is keyed
    // 1st by plant_id and then by Group||cvterm_id (ie: B4243).
    // Note: this first array will not have any missing data cells filled in.
    $rows = array();
    foreach($results as $r) {
      $rows[ $r->id ][ $r->grp . $r->tid ] = $r->value;
    }
    
    // Create a drupal managed file to write to. We need to create a managed file in order
    // to use file_transfer to give the file to the user securely. However, we cant use
    // fputcsv() with file_save_data() (typical way to create file for download). Thus
    // we are essentially duplicating fiel_dave_data() functionality here :-(.
    global $user;
    $filename = 'rawpheno_csv' . date('YMd') .'_'. time() . '.csv';
    $uri = drupal_tempnam('temporary://', 'file');
    $file = new stdClass();
    $file->fid = NULL;
    $file->uri = $uri; // This is where the file actually is.
    $file->filename = $filename; //This is what we want the file to be named when downloaded.
    $file->filemime = 'text/csv';
    $file->uid = $user->uid;
    file_save($file);
    // Now open the file for writting.
    $OUT = fopen($uri, 'w');
    
    // Now write the header to the file.
    fputcsv($OUT, $header);
    
    // Now we want to process each row to add entries for missing data.
    // We will do this by looping through the header for each row and if there isn't
    // already data set for that cell then we will add missing data.
    // Note: We will save the data to the file as we go.
    foreach ($rows as $row) {
    
      // First, add missing data for each entry in the header that is not in the row.
      foreach ($header as $id => $title) {
        if (!isset($row[$id])) {
          $row[$id] = 'NA';
        }
      }
      
      // Sort the row based on the keys to make sure they match with the header.
      ksort($row);
      
      // Write this row to the file as CSV.
      fputcsv($OUT, $row);
    }
  }
  
  fclose($OUT);
  
  // Notify the drupal filesystem of the size change.
  $file->filesize = filesize(drupal_realpath($file->uri));
  file_save($file);

  // Set page headers.
  $http_headers = array(
    'Content-Type' => 'text/csv',
    'Content-Disposition' => 'attachment; filename="' . $filename . '"',
    'Content-Length' => filesize(drupal_realpath($file->uri)),
  );

  if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
    $http_headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
    $http_headers['Pragma'] = 'public';
  }
  else {
    $http_headers['Pragma'] = 'no-cache';
  }

  // Call drupal file_transfer() function.
  file_transfer($uri, $http_headers);
}


/**
 * Implements hook_validate().
 */
function rawpheno_download_validate($form, &$form_state) {
  $locations = $form_state['values']['sel_location'];
  $traits = $form_state['values']['sel_trait'];
  
  if (count($traits) <= 0) {
    // No trait selected.
    form_set_error('sel_trait', t('Please select traits and try again.'));
  }
  
  if (count($locations) <= 0) {
    // No location selected.
    form_set_error('sel_location', t('Please select location and try again.'));
  }
}


/**
 * Function get the R Compatible version of a trait.
 *
 * @param:
 *  $cvterm_id a numeric value containing the cvterm id number of a given trait.
 * @return
 *  A string containing the R compatible version of the trait.
 */
function rawpheno_download_r_compatible($cvterm_id) {
  $cv_rver = tripal_get_cv(array('name' => 'phenotype_r_compatible_version'));

  $sql = "SELECT value FROM {cvtermprop} WHERE cvterm_id = :cvterm_id AND type_id = :cv_id LIMIT 1";
  $args = array(':cvterm_id' => $cvterm_id, 'cv_id' => $cv_rver->cv_id);
  
  $r = chado_query($sql, $args)
    ->fetchField();

  return (isset($r) AND !empty($r)) ? $r : null;
}