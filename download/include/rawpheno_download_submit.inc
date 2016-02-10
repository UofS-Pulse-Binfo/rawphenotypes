<?php
 
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
  $file_name = 'rawpheno_csv' . date('d-m-Y') . time() . '.csv';
  $file_csv = file_save_data(implode(',', $arr_headers) . "\n" .
                             implode("\n", $arr_csv_rows), 'public://' . $file_name);
 
  // Include directory path to the filename.
  $path = 'public://'.$file_name;
 
  // Set page headers.
  $http_headers = array(
    'Content-Type' => 'text/csv',
    'Content-Disposition' => 'attachment; filename="' . 'rawdata.csv' . '"',
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