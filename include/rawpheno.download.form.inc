<?php

/**
 * @file
 * Contains all implementations of hook_form().
 */

/**
 * Function callback: construct export data form.
 */
function rawpheno_download($form, &$form_state) {
  // This is the error message used in lieu of Drupal form_set_error().
  // This error message will be in close proximity to the interface.
  $form['download_window_error'] = array(
    '#markup' => t('<div id="download-window-error" class="messages error"></div>'),
  );
  
  // Navigation button. Related page of download page is rawdata/summary page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('View Summary &#10095;'),
  );  
  
  // Primary select field - an AJAX chained select that updates secondary
  // select field (traits) based on location selected.
  // Default to the first location in the list.
  // Query all locations available.
  $location = db_query("SELECT DISTINCT t1.value, t1.value AS location
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t2.name = 'location'
    ORDER BY t1.value ASC")
    ->fetchAllKeyed();  

  $form['location'] = array(
    '#title' => t('Location'),
    '#type' => 'select',
    '#options' => $location,
    '#default_value' => reset($location),
    '#multiple' => TRUE,
    '#size' => 17,
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_measurements',
      'wrapper' => 'div-measurements',
      'progress' => array('type' => '', 'message' => '')
    ),
  );
  
  // Secondary select field - list of traits avaliable for download.
  // Query all traits available per location.
  // Note: planting date is excluded from the list in the same manner it is
  // excluded from the rawdata heat map.
  // Determine default traits to display based on value of location.
  if (isset($form_state['values']['location'])) {
    // When user has selected a location.
    $loc = $form_state['values']['location'];
  }
  else {
    // No selection made or on page load - default location to the first
    // location available in primary select field.
    $loc = reset($location);
  }
  
  $traits = db_query("SELECT DISTINCT t1.type_id, t2.definition
    FROM {pheno_measurements} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN
      (SELECT DISTINCT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t1.value <> ''
      AND t2.name <> 'planting_date'
    ORDER BY t1.type_id ASC", array(':location' => $loc))
    ->fetchAllKeyed();
  
  // Div wrapper to display ajax result.
  $form['ajax_wrapper'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="div-measurements">',
    '#suffix' => '</div>'
  );

  $form['ajax_wrapper']['traits'] = array(
    '#title' => t('@traits-count Traits available', array('@traits-count'=> count($traits))),
    '#type' => 'select',
    '#options' => $traits,
    '#multiple' => TRUE,
    '#size' => 17,
  );
  
  // Checkbox to select all traits.
  $form['ajax_wrapper']['chk_select_all'] = array(
    '#title' => t('Select all traits'),
    '#type' => 'checkbox',
    '#default_value' => 0,
  );
  
  $form['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
  );
  
  // Attach CSS and JavaScript
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.download.style.css');
  $form['#attached']['js']  = array($path . 'js/rawpheno.download.script.js');
    
  return $form;
}

/**
 * Function callback: set form wrapper for AJAX.
 */
function rawpheno_download_measurements($form, $form_state) {
  return $form['ajax_wrapper'];
}

/**
 * Implements hook_form_submit().
 *
 * Generate a comma separated values (csv) file based on the location and trait set selected.
 */
function rawpheno_download_submit($form, &$form_state) {
  // Delay function not to startle user with dialog box to save file.
  // A counter in front-end in sync whith this delay.
  sleep(4);
  
  // Ensure that public directory exists.
  $csv_path = 'public://';
  if (!file_prepare_directory($csv_path, FILE_CREATE_DIRECTORY)) {
    drupal_set_message(t('Unable to find public directory. Please check directory and try again.'), 'error');
    return FALSE;
  }

  // Ensure the public directory is writable.
  $csv_filename = 'rawphenotypes-' . REQUEST_TIME . '.csv';
  $csv_file = file_save_data('', $csv_path . $csv_filename);
  if (!$csv_file) {
    drupal_set_message(t('Unable to create file in public directory. Please check directory permission and try again.'), 'error');
    return FALSE;
  }
  
  // Append file with data in csv format.
  $csv_file_open = fopen($csv_file->uri, 'w');
  /////
  
  // Location or locations selected by user.
  $location = $form_state['values']['location'];
  // List of traits selected by user.
  $traits = $form_state['values']['traits'];
  
  // Add planting date to the list of traits.
  $planting_date = tripal_get_cvterm(array('name' => 'planting_date'));
  array_push($traits, $planting_date->cvterm_id);
  
  // Query all traits and location based on location and traits selected by user.
  // The result follows a standard format: PLOT, ENTRY, NAME, REP, LOCATION, ...
  $sql = db_query("SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'a' as grp
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t2.name IN ('plot', 'entry')
    UNION
    SELECT t2.plant_id AS id, '0' as tid, 'Name' AS def, t1.name AS data, 'b' as grp
    FROM {chado.stock} AS t1 
      INNER JOIN {pheno_plant} AS t2 USING(stock_id)
    WHERE t2.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
    UNION
    SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'c' as grp
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t2.name IN ('rep', 'location')
    UNION
    SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'd' as grp
    FROM {pheno_measurements} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t2.cvterm_id IN (:traits)
    ORDER BY id, grp, tid ASC", array(':location' => $location, ':traits' => $traits));
    
    if ($sql) {
      // Array to hold column headers.
      $arr_download_data = array();
      // Index 0 to hold all column headers.
      $arr_download_data[0] = array();
      while($row = $sql->fetchAssoc()) {
        // Column headers
        if (!in_array($row['def'], $arr_download_data[0])) {
          $arr_download_data[0][] = $row['def'];
        }
        
        // Data rows.
        // Determine if data has a value or null.
        // Assign NA - Not Applicable when data is NULL.
        $arr_download_data[$row['id']][] = empty($row['data']) ? 'NA' : $row['data']; 
      }
    }
  
  /////
  foreach($arr_download_data as $data) {
    fputcsv($csv_file_open, $data);
  }
  
  fclose($csv_file_open);
  
  // Notify the drupal filesystem of the size change.
  $csv_file->filesize = filesize($csv_file->uri);
  file_save($csv_file);

  // Set page headers.
  $http_headers = array(
    'Content-Type' => 'text/csv',
    'Content-Disposition' => 'attachment; filename="' . $csv_filename . '"',
    'Content-Length' => $csv_file->filesize,
  );  

  if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
    $http_headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
    $http_headers['Pragma'] = 'public';
  }
  else {
    $http_headers['Pragma'] = 'no-cache';
  }

  // Call drupal file_transfer() function.
  file_transfer($csv_file->uri, $http_headers);
}

/**
 * Implements hook_validate().
 */
function rawpheno_download_validate($form, &$form_state) {
  // Validate location.
  $locations = $form_state['values']['location'];
  if (count($locations) <= 0) {
    // No location selected.
    form_set_error('location', t('No location selected. Please select location and try again.'));
  }

  // Validate traits.
  $traits = $form_state['values']['traits'];
  if (count($traits) <= 0) {
    // No trait selected.
    form_set_error('traits', t('No traits selected. Please select traits and try again.'));
  }
}