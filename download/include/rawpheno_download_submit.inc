<?php
 
/**
 * Implements a hook_form_submit()
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_download_submit($form, &$form_state) {
  //Delay function not startle user with dialog box to save file.
  //A counter in front-end in sync whith this delay.
  sleep(4);
 
  //Location selected by user.
  $location = $form_state['values']['location'];
  $location = "'".implode("','", $location)."'";
 
  //list of traits selected by user.
  $traits = $form_state['values']['traits'];
  $traits = implode(',', $traits);
 
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
 
  //Create csv data
  if ($d) {
    $data_header = array();
    $data_row = array();
    while($r = $d->fetchAssoc()) {
      if (!in_array($r['def'], $data_header)) {
        array_push($data_header, $r['def']);   
      }
      
      $m  = (empty($r['data'])) ? 'NA' : $r['data'];
      $data_row[$r['id']][] = trim($m);
    }
  }
 
  $traits = array();
  foreach($data_row as $i=>$h) {
    $o = array_splice($h, 0, 1);
    array_splice($h, 2, 0, $o);
    $traits[] = implode(',', $h);
  }
 
  $out = array_splice($data_header, 0, 1);
  array_splice($data_header, 2, 0, $out);

  //file  
  $file_name = 'rawpheno_csv' . date('d-m-Y') . time() . '.csv';
  $file_csv = file_save_data(implode(',', $data_header) . "\n" .
                             implode("\n", $traits), 'public://' . $file_name);
 
  //set header and save file
  $path = 'public://'.$file_name;
 
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

  file_transfer($path, $http_headers);
}