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
  $h = 'Please review form for any empty fields';
  $form['download_window_error'] = array(
    '#markup' => '<div id="download-window-error" class="messages error">Required fields empty.<pre><h3>'.$h.
                 '</h3></pre></div>',
  );
  
  // Fieldset container for download form.
  $form['download_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select locations and traits that you want to download'),
    '#attributes' => array('id' => 'field-container'),
  );
  
  // Query all locations available.
  $location = db_query("SELECT DISTINCT t1.value, t1.value AS location 
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t2.name = 'location' 
    ORDER BY t1.value ASC")
    ->fetchAllKeyed();
  
  // Primary select field - an AJAX chained select that updates secondary 
  // select field (traits) based on location selected. 
  // Default to the first location in the list.
  $form['download_fldset']['location'] = array(
    '#title' => t('Location'),
    '#type' => 'select',
    '#options' => $location,
    '#default_value' => reset($location),
    '#multiple' => TRUE,
    '#size' => 17,
    '#prefix' => '<div class="frm-cell">',
    '#suffix' => '</div>',
    '#ajax' => array(
      'event' => 'change',
      'callback' => 'rawpheno_download_measurements',
      'wrapper' => 'div-measurements',
      'progress' => array('type' => '', 'message' => '')
    ),
  );
  
  // Div wrapper to display ajax result.
  $form['download_fldset']['wrapper'] = array(
    '#prefix' => '<div id="div-measurements">',
    '#suffix' => '</div>'
  );
  
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
  
  // Query all traits available per location.
  // Note: planting date is excluded from the list in the same manner it is
  // excluded from the rawdata heat map.
  $traits = db_query("SELECT DISTINCT t1.type_id, t2.definition 
    FROM {pheno_measurements} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN 
      (SELECT DISTINCT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t1.value <> ''
      AND t2.name <> 'planting_date'
    ORDER BY t1.type_id ASC", array(':location' => $loc))
    ->fetchAllKeyed();
    
  // Secondary select field - list of traits avaliable for download.
  $form['download_fldset']['wrapper']['traits'] = array(
    '#title' => count($traits).' Traits available',
    '#type' => 'select',
    '#options' => $traits,
    '#multiple' => TRUE,
    '#size' => 17,
  );
  
  // Submit button.
  $form['download_fldset']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#prefix' => '<div class="frm-cell"><input type="checkbox"> Select all traits',
    '#suffix' => '</div><div style="clear: both;"></div>',
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
  return $form['download_fldset']['wrapper'];
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
 
  // Location or locations selected by user.
  $location = $form_state['values']['location'];
  // List of traits selected by user.
  $traits = $form_state['values']['traits'];
  
  // Add planting date to the list of traits.
  $planting_date = tripal_get_cvterm(array('name' => 'planting_date'));
  array_push($traits, $planting_date->cvterm_id);
  
  // Query to join data from different tables.
  // Result: plant_id, trait_id, definition, data, and a grouping string
  // The result is sorted by plant_id and the grouping string ensuring that the first
  // row is Name - containing the stock name.
  // The result will be sorted into standard order: plot,entry,name,rep,location,traits.....
  $sql = db_query("SELECT t2.plant_id AS id, '0' as tid, 'Name' AS def, t1.name AS data, 'A' as grp
    FROM {chado.stock} AS t1 
      INNER JOIN {pheno_plant} AS t2 USING(stock_id)
    WHERE t2.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
    UNION
    SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'B' as grp
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
    UNION
    SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'C' as grp
    FROM {pheno_measurements} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t2.cvterm_id IN (:traits)
    ORDER BY id, grp, tid ASC", array(':location' => $location, ':traits' => $traits));
 
  // Create header row and data row of the csv file.
  if ($sql) {
    // Array to hold column headers.
    $arr_headers = array();
    // Array to hold data rows
    $arr_data_rows = array();
    
    while($row = $sql->fetchAssoc()) {
      // Determine if the column header is in the array of column headers
      if (!in_array($row['def'], $arr_headers)) {
        // Not in the array of column headers. - Add this header.
        array_push($arr_headers, $row['def']);   
      }
      
      // Determine if data has a value or not.
      // Assign NA - Not Applicable when data is NULL
      $value = (empty($row['data'])) ? 'NA' : trim($row['data']);
      // Create an array of data with plant_id as the index.
      $plant_id = $row['id'];
      $arr_data_rows[$plant_id][] = $value;
    }
  }
  
  // Array to hold the string-comma-separated values 
  $arr_csv_rows = array();
  // Read each rows of data.
  foreach($arr_data_rows as $index => $value) {
    // Splice name element.
    $name_element = array_splice($value, 0, 1);
    // Insert name data in between entry and rep.
    array_splice($value, 2, 0, $name_element);
    // Convert the array into string-comma-separated.
    $arr_csv_rows[] = implode(',', $value);
  }
 
  // Splice name header.
  $name_header = array_splice($arr_headers, 0, 1);
  // Insert in between entry and rep.
  array_splice($arr_headers, 2, 0, $name_header);

  // Create filename, with date and time information.  
  $file_name = 'rawpheno_csv' . date('YMd') .'_'. time() . '.csv';
  $file_csv = file_save_data(implode(',', $arr_headers) . "\n" .
                             implode("\n", $arr_csv_rows), 'public://' . $file_name);
 
  // Include directory path to the filename.
  $path = 'public://'.$file_name;
 
  // Set page headers.
  $http_headers = array(
    'Content-Type' => 'text/csv',
    'Content-Disposition' => 'attachment; filename="' . $file_name . '"',
    'Content-Length' => filesize($path),
  );

  if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
    $http_headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
    $http_headers['Pragma'] = 'public';
  }
  else {
    $http_headers['Pragma'] = 'no-cache';
  }

  // Call drupal file_transfer() function.
  file_transfer($path, $http_headers);
}

/**
 * Implements hook_validate().
 */
function rawpheno_download_validate($form, &$form_state) {
  $traits = $form_state['values']['traits'];
  if (count($traits) <= 0) {
    // No trait select.
    form_set_error('traits', t('Please select traits and try again.'));
  }
}
