<?php

/**
 * @file
 * implements hook form in phenotypes/download page
 */
 
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_submit');
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_validate');

/**
 * menu callback for rawpheno download
 *
 * @param type $form
 * @param type $form_state
 */
function rawpheno_download($form, &$form_state) {
  if (!isset($form_state['stage'])) {
    $form_state['stage'] = 'form';
  }
  
  $form = array();
  switch($form_state['stage']) {
    case 'form':
      //display form elements
      return rawpheno_download_traits_form($form, $form_state);
      break;

    case 'download':
      //generate csv and download 
      return rawpheno_download_autodownload_form($form, $form_state);
      break;
  }

  return $form;
}

/**
 * rawpheno download select traits form
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_download_traits_form($form, $form_state) {
  $form['download_fldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Select location and traits that you want to download'),
    '#attributes' => array('id' => 'field-container'),
  );
  
  //populate locations
  $sql = "SELECT DISTINCT t1.value, t1.value as location
          FROM {pheno_plantprop} as t1 INNER JOIN {chado.cvterm} as t2
          ON t1.type_id = t2.cvterm_id
          WHERE t2.name = 'location'
          ORDER BY location ASC";        

  $location = db_query($sql)->fetchAllKeyed();
 
  //primary select
  //container div to contain all the form elements
  $form['download_fldset']['location'] = array(
    '#type' => 'select',
    '#title' => t('Location'),
    '#size' => 15,
    '#options' => $location,
    '#default_value' => reset($location),
    '#prefix' => '<div class="frm-cell">',
    '#suffix' => '</div>',
  );

  //wrap select in div
  $form['download_fldset']['begin_div'] = array('#markup' => '<div class="frm-cell">');  
  //create chained select of traits per location
  foreach($location as $key => $val) {
    //select all available traits for download
    $sql = "SELECT DISTINCT t1.type_id AS trait_id, t3.definition AS trait
            FROM {pheno_measurements} AS t1 
            INNER JOIN {pheno_plantprop} AS t2 ON t1.plant_id = t2.plant_id
            INNER JOIN {chado.cvterm} AS t3 ON t1.type_id = t3.cvterm_id
            WHERE t2.value  = '$key' and t1.value <> ''
            ORDER BY t1.type_id ASC";        
 
    $traits = db_query($sql)->fetchAllKeyed();
    
    $form['download_fldset']['sel-'.$key] = array(
      '#type' => 'select',
      '#title' => count($traits).' Traits available for location '.$val,
      '#states' => array(
          'visible' => array(
              ':input[name="location"]' => array('value' => $key),
          ),
      ),
      '#options' => $traits,
      '#size' => 15,
      '#multiple' => TRUE,
    );
  }
  $form['download_fldset']['end_div'] = array('#markup' => '</div>');

  //submit button
  $form['download_fldset']['download_submit_download'] = array(
    '#type' => 'submit',
    '#value' => 'Download',
    '#prefix' => '<div class="frm-cell"><input type="checkbox"> Select all traits',
    '#suffix' => '</div><div style="clear: both;"></div>',
  );   
  
  //attach css and js
  $form['#attached']['css'] = array(drupal_get_path('module', 'rawpheno') . '/download/css/rawpheno_download_style.css');
  $form['#attached']['js'] = array(drupal_get_path('module', 'rawpheno') . '/download/js/rawpheno_download_stage01.js');
  
  return $form;
}

/**
 * rawpheno download autodownload CSV file
 *
 * @param type $form
 * @param type $form_state
 * @return type
 */
function rawpheno_download_autodownload_form($form, $form_state) {
  //delay download for 4 sec
  sleep(4);
  
  //create csv file for download
  $location = $form_state['values']['location'];
  $select = 'sel-'.$location;
  $traits = $form_state['values'][$select];
  //include plantprop traits
  $plantprop_traits = "'plot','entry','rep','location'";
  //list of traits to search
  $traits = implode(',', $traits);
  
  //get stock name of the plant id based on given location
  $n = db_query("SELECT t2.name, t1.plant_id, t3.value AS location
                 FROM {pheno_plant} AS t1
                  INNER JOIN {chado.stock} AS t2 USING(stock_id)
                  INNER JOIN {pheno_plantprop} AS t3 ON t1.plant_id = t3.plant_id
                 WHERE t3.value = '$location'
                 ORDER BY t1.plant_id ASC");
  
  if ($n) {
    //create the headers - Name is inserted after rep
    //order is based on the selected traits
    $h = db_query("SELECT ARRAY_TO_STRING(ARRAY_AGG(CASE WHEN name = 'rep' 
                   THEN CONCAT(definition, ',Name') ELSE TRIM(definition) END), ',') 
                   FROM {chado.cvterm}
                   WHERE name IN ($plantprop_traits) OR cvterm_id IN ($traits)");

    $h = $h->fetchField()."\n";
 
    $raw = '';
    while($row = $n->fetchAssoc()) {
      //each measurements based on plant_id
      
      $plant_id = $row['plant_id'];
      $stock = $row['name'];
      
      //plantprop in this order (plot, entry, name, rep, location) 
      $p = db_query("SELECT ARRAY_TO_STRING(ARRAY_AGG(CONCAT(
                      CASE WHEN t2.name = 'entry' THEN t1.value ELSE null END,
                      CASE WHEN t2.name = 'plot' THEN t1.value ELSE null END,
                      CASE WHEN t2.name = 'rep' THEN concat(t1.value, ',$stock') ELSE null END,
                      CASE WHEN t2.name = 'location' THEN concat(t1.value,',') ELSE null END
                      )),',') as plantprop
                     FROM pheno_plantprop AS t1 
                      INNER JOIN chado.cvterm AS t2 ON t1.type_id = t2.cvterm_id
                     WHERE t1.plant_id = '$plant_id'
                     GROUP BY t1.plant_id LIMIT 1");
      
      $raw .= $p->fetchField();
  
      //measurement values - in the same order as selected traits
      $m = db_query("SELECT array_to_string(array_agg(CASE WHEN t1.value = '' THEN 'NA' ELSE trim(t1.value) END), ',') as value
                     FROM {pheno_measurements} as t1
                     WHERE t1.type_id IN ($traits) AND t1.plant_id = '$plant_id'
                     GROUP BY t1.plant_id");
      
      $raw .= $m->fetchField()."\n"; 
    }    
  }
  
  //file  
  $file_name = 'rawpheno' . $location.'csv' . time() . '.csv';
  $file_csv = file_save_data($h.$raw, 'public://' . $file_name);
  
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
  
  return $form;
}