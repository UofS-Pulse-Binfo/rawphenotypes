<?php
 
/**
 * Implements hook_form_submit().
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_download_submit($form, &$form_state) {
  //Delay function not startle user with dialog box to save file.
  //A counter in front-end in sync whith this delay.
  sleep(4);
 
  //Location or locations selected by user.
  $location = $form_state['values']['location'];
  $location = "'".implode("','", $location)."'";
 
  //list of traits selected by user.
  $traits = $form_state['values']['traits'];
  $traits = implode(',', $traits);
 
  //Query to join data from different tables.
  //Result: plant_id, trait_id, definition, data, and a grouping string
  //The result is sorted by plant_id and the grouping string ensuring that the first
  //row is Name - containing the stock name.
  //The result will be sorted into standard order: plot,entry,name,rep,location,traits.....
  //when creating the cvs file.
  $d = db_query("SELECT t2.plant_id AS id, '0' as tid, 'Name' AS def, t1.name AS data, 'A' as grp
                 FROM {chado.stock} AS t1
                   INNER JOIN {pheno_plant} AS t2 USING(stock_id)
                 WHERE t2.plant_id IN
                 (SELECT plant_id FROM {pheno_plantprop} WHERE value IN ($location))
                 UNION
                 SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'B' as grp
                 FROM {pheno_plantprop} AS t1
                   INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
                 WHERE t1.plant_id IN
                 (SELECT plant_id FROM {pheno_plantprop} WHERE value IN ($location))
                 UNION
                 SELECT t1.plant_id AS id, t1.type_id as tid, t2.definition AS def, t1.value AS data, 'C' as grp
                 FROM {pheno_measurements} AS t1
                   INNER JOIN {chado.cvterm} AS t2 ON t1.type_id = t2.cvterm_id
                 WHERE t1.plant_id IN
                 (SELECT plant_id FROM {pheno_plantprop} WHERE value IN ($location))
                 AND cvterm_id IN ($traits)
                 ORDER BY id, grp, tid ASC");
 
  //Create header row and data row of the csv file.
  if ($d) {
    //Array to hold column headers.
    $data_header = array();
    //Array to hold data rows
    $data_row = array();
    
    while($r = $d->fetchAssoc()) {
      //Determine if the column header is in the array of column headers
      if (!in_array($r['def'], $data_header)) {
        //Not in the array of column headers. - Add this header.
        array_push($data_header, $r['def']);   
      }
      
      //Determine if data has a value or not.
      //Assign NA - Not Applicable when data is NULL
      $m  = (empty($r['data'])) ? 'NA' : $r['data'];
      //Create an array of data with plant_id as the index.
      $data_row[$r['id']][] = trim($m);
    }
  }
  
  //Array to hold the string-comma-separated values 
  $traits = array();
  //Read each rows of data.
  foreach($data_row as $i=>$h) {
    //Splice name element.
    $name_element = array_splice($h, 0, 1);
    //Insert name data in between entry and rep.
    array_splice($h, 2, 0, $name_element);
    //Convert the array into string-comma-separated.
    $traits[] = implode(',', $h);
  }
 
  //Splice name header.
  $name_header = array_splice($data_header, 0, 1);
  //Insert in between entry and rep.
  array_splice($data_header, 2, 0, $name_header);

  //Create filename, with date and time information.  
  $file_name = 'rawpheno_csv' . date('d-m-Y') . time() . '.csv';
  $file_csv = file_save_data(implode(',', $data_header) . "\n" .
                             implode("\n", $traits), 'public://' . $file_name);
 
  //Include directory path to the filename.
  $path = 'public://'.$file_name;
 
  //Set page headers.
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

  //Call drupal file_transfer() function.
  file_transfer($path, $http_headers);
}