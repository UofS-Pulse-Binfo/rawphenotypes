<?php

/**
 * @file
 * implements hook form in phenotypes/download page
 */
 
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_submit');
module_load_include('inc', 'rawpheno', 'download/include/rawpheno_download_validate');

//button labels
define('RAWPHENO_DOWNLOAD_BTN_DOWNLOAD', 'Download');
define('RAWPHENO_DOWNLOAD_BTN_DOWNLOAD_02', 'Download another file');
define('RAWPHENO_DOWNLOAD_BTN_HOME', 'Go to home KnowPulse');

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
            ORDER BY trait ASC";        
 
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
    '#value' => RAWPHENO_DOWNLOAD_BTN_DOWNLOAD,
    '#prefix' => '<div class="frm-cell"><input type="checkbox"> Select all traits',
    '#suffix' => '</div><div style="clear: both;"></div>',
  );   
  
  //attach css and js
  $form['#attached']['css'] = array(RAWPHENO_PATH . '/download/css/rawpheno_download_style.css');
  $form['#attached']['js'] = array(RAWPHENO_PATH . '/download/js/rawpheno_download_stage01.js');
  
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
  //list of traits to search
  $traits = implode(',', $traits);
  
  //headers
  $headers = db_query("SELECT array_to_string(array_agg(trim(definition)), ',') 
                       FROM {chado.cvterm}
                       WHERE cvterm_id IN ($traits)");
  
  $h = $headers->fetchField()."\n";
  
  //all data
  $data = db_query("SELECT array_to_string(array_agg(trim(t1.value)), ',') as value
                    FROM {pheno_measurements} AS t1 
                    INNER JOIN {pheno_plantprop} AS t2 ON t1.plant_id = t2.plant_id
                    WHERE t1.type_id IN ($traits) AND t2.value = '$location'
                    GROUP BY t1.plant_id");
                    
  if ($data) {
    $d = '';
    while($row = $data->fetchAssoc()) {
      $d .= $row['value']."\n";
    }
  }
  
  //file  
  $file_name = 'rawpheno' . $location.'csv' . time() . '.csv';
  $file_csv = file_save_data($h.$d, 'public://' . $file_name);
  
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