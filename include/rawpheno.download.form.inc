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
    '#markup' => t('<div id="download-window-error" class="messages error">
                 No traits or location selected. Please try again.</div>'),
  );
  
  // Navigation button. Related page of download page is rawdata/summary page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('View Summary <span>&#10095;</span>'),
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
  
  // First we need to get the header. This will allow us to ensure that the data
  // downloaded all matches up with the trait it is associated with. Furthermore,
  // it will allow us to handle missing data.
  $sql = "SELECT 'B'||t2.cvterm_id as id, t2.definition 
            FROM pheno_plantprop t1 
            LEFT JOIN chado.cvterm t2 ON t2.cvterm_id=t1.type_id 
            WHERE t1.plant_id IN (SELECT plant_id FROM pheno_plantprop WHERE value IN (:location))
        UNION
          SELECT 'C'||t2.cvterm_id as id, t2.definition 
            FROM pheno_measurements t1 
            LEFT JOIN chado.cvterm t2 ON t2.cvterm_id=t1.type_id 
            WHERE t1.plant_id IN (SELECT plant_id FROM pheno_plantprop WHERE value IN (:location)) 
              AND t2.cvterm_id IN (:traits)
        GROUP BY t2.cvterm_id, t2.definition";
  $result = db_query($sql, array(':location' => $location, ':traits' => $traits))->fetchAll();
  $header = array('A0' => 'Name');
  foreach ($result as $r) {
    $header[ $r->id ] = $r->definition;
  }
  ksort($header);

  
  // Query to join data from different tables.
  // Result: plant_id, trait_id, definition, data, and a grouping string
  // The result is sorted by plant_id and the grouping string ensuring that the first
  // row is Name - containing the stock name.
  // The result will be sorted into standard order: plot,entry,name,rep,location,traits.....
  // Thus first we select the name. Note that  the tid is 0 because this doesn't have a cvterm (ie: not a trait).
  $sql = "SELECT t2.plant_id AS id, '0' as tid, 'Name' AS def, t1.name AS value, 'A' as grp
    FROM {chado.stock} AS t1 
      INNER JOIN {pheno_plant} AS t2 USING(stock_id)
    WHERE t2.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))"
    // Then we add on the required information (ie: plot, entry, rep, location)
    ." UNION
    SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS value, 'B' as grp
    FROM {pheno_plantprop} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))"
    // Finally we add in all the traits.
    ."UNION
    SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS value, 'C' as grp
    FROM {pheno_measurements} AS t1
      INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
    WHERE t1.plant_id IN (SELECT plant_id FROM {pheno_plantprop} WHERE value IN (:location))
      AND t2.cvterm_id IN (:traits)"
    // Lastly we order the results by plant_id and grouping string, and tid.
    ."ORDER BY id, grp, tid ASC";
  $results = db_query($sql, array(':location' => $location, ':traits' => $traits));

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
    $file->filemime = file_get_mimetype($file->uri);
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
  $file->filesize = filesize($file->uri);
  file_save($file);

  // Set page headers.
  $http_headers = array(
    'Content-Type' => 'text/csv',
    'Content-Disposition' => 'attachment; filename="' . $filename . '"',
    'Content-Length' => filesize($uri),
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
  $traits = $form_state['values']['traits'];
  $locations = $form_state['values']['location'];
  
  if (count($traits) <= 0) {
    // No trait selected.
    form_set_error('traits', t('Please select traits and try again.'));
  }
  
  if (count($locations) <= 0) {
    // No location selected.
    form_set_error('location', t('Please select location and try again.'));
  }
}