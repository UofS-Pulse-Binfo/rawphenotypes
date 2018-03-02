<?php
/**
 * @file
 * TRIPAL DOWNLOAD API IMPLEMENTATION (Function Callback).
 */

/**
 *  Implements hook_register_trpdownload_type().
 */
function rawpheno_register_trpdownload_type() {
  $types = array();

  // The key is the machine name of my download type.
  $types['rawpheno_csv'] = array(
    // A human readable name to show in an administrative interface one day.
    'type_name' => 'Raw Phenotypes CSV',
    // A human readable description of the format.
    'format' => 'Comma-separated Values',
    // An array of functions that the API will use to customize your experience.
    'functions' => array(
      // The function that tripal jobs will call to generate the file.
      'generate_file' => 'rawpheno_trpdownload_generate_file',
      // OPTIONAL: provide a summary to the user on the download page.
      'summarize' => 'rawpheno_trpdownload_summarize_download',
      // OPTIONAL: determine your own filename.
      'get_filename' => 'rawpheno_trpdownload_get_filename',
      // OPTIONAL: Change the file suffix (defaults to .txt)
      'get_file_suffix' => 'rawpheno_trpdownload_get_suffix',
      // OPTIONAL: determine the human-readable format based on a function.
      'get_format' => 'rawpheno_trpdownload_get_readable_format',
    ),
  );

  return $types;
}

/**
 * Generate a file summary.
 */
function rawpheno_trpdownload_summarize_download($vars) {
  // Path to module.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  // Style nav links.
  $style = 'float: left; padding: 20px 0 0 0; height: 50px; font-size: 1em; margin: 0 2.5%; text-align:center; width: 20%; color: #118324 !important';
  // Style span element.
  $style2 = 'background-color: #FFFFFF; display: block; margin-top: 24px; padding: 5px 0;';

  // Div container to hold summary of options selected by user.
  $summary = '';

  // PROJECTS:
  $q = trim(implode('', $vars['download_args']['q']));
  $q = base64_decode($q);
  list($project, $location, $traits, $r_version, $envdata, $envfile) = explode('&', $q);

  $tmp = trim(str_replace('p=', '', $project));
  $project = explode(',', $tmp);

  $sql = "SELECT name FROM {project} WHERE project_id IN (:project)";
  $args = array(':project' => $project);
  $p = chado_query($sql, $args)
    ->fetchAllKeyed(0,0);
  $summary .= '<li> ' . count($p) . ' PROJECTS: <br /><em>' . implode(', ', $p) . '</em></li>';

  // LOCATIONS:
  $location = trim(str_replace('l=', '', $location));

  if ($location == '0') {
    // Load all locations available.
    $sql = "SELECT DISTINCT value AS location
            FROM pheno_plantprop
            WHERE type_id = (SELECT cvterm_id FROM {cvterm} cvt LEFT JOIN {cv} cv ON cv.cv_id=cvt.cv_id WHERE cvt.name = 'Location' AND cv.name = 'phenotype_plant_property_types')
            ORDER BY value ASC";
    $location = chado_query($sql)
      ->fetchCol(0);
  }
  else {
    $location = explode(',', $location);
  }
  $summary .= '<li> ' . count($location) . ' LOCATIONS: <br /><em>' . implode(', ', $location) . '</em></li>';

  // TRAITS:
  $traits = trim(str_replace('t=', '', $traits));

  if ($traits == 0) {
    // Load all traits available.
    $t = rawpheno_download_load_traits($location, $project);
    $traits = array_keys($t);
  }
  else {
    $traits = explode(',', $traits);
  }

  $sql = "SELECT name FROM {cvterm} WHERE cvterm_id IN (:cvterm_id)";
  $args = array(':cvterm_id' => $traits);
  $t = chado_query($sql, $args)
    ->fetchAllKeyed(0,0);
  $summary .= '<li> ' . count($t) . ' TRAITS: <br /><em>' . implode(', ', $t) . '</em></li>';

  // RVERSION:
  $r_version = trim(str_replace('r=', '', $r_version));

  $r_ver = ($r_version == 1) ? 'Yes' : 'No';
  $summary .= '<li>R FRIENDLY: <br /><em>' . $r_ver . '</em></li>';

  // INCLUDE ENVIRONMENT DATA.
  $envdata = trim(str_replace('e=', '', $envdata));
  $e_data = ($envdata == 1) ? 'Yes' : 'No';
  $summary .= '<li>INCLUDE ENVIRONMENT DATA: <br /><em>' . $e_data . '</em></li>';

  $output = '<div style="overflow-y: scroll; height: 60px; border: 1px solid #EAEAEA; padding: 5px 20px; background-color: #FFFFFF">' . $summary . '</div>';

  // Environment data files.
  if ($envdata == 1 && $envfile !== 0) {
    $envfile = trim(str_replace('file=', '', $envfile));

    $file_icon = theme_image(array(
      'path' => drupal_get_path('module','trpdownload_api').'/theme/icons/file_generic.128.png',
      'alt' => 'download environment data file',
      'attributes' => array()
    ));

    // File type:
    $file_type = (strpos($envfile, 'tar')) ? 'TAR Archive File' : 'Microsoft Excel File';
    // File link:
    $file_link = l($envfile,
      file_create_url('public://') . $envfile,
      array('attributes' => array('download' => $envfile, 'target' => '_blank'))
    );

    $output .= '<div style="position: absolute; left: 0; width: 100%; margin-top: 40px; padding: 10px;">
      <div style="margin: 0 20px 0 0;">
        <div class="download-pane" style="margin: 0 20px;">'

         . $file_icon .

         '<div class="inner-pane file">
            <h2>Environment Data File:</h2>
            <div class="file-link">' . $file_link . '</div>
            <div class="file-format">Format:' . $file_type . '</div>
          </div>
        </div>
      </div>
    </div>';

    $margin = '200px';
  }
  else {
    $margin = '30px';
  }

  // Div container to hold navigation links to rawphenotypes page.
  $output .= '<div style="position: absolute; margin-top: ' . $margin . '; display: inline-block; left: 12%; width: 70%; text-align: center;">
    <div class="container-buttons" style="padding: 30px 0 0 0; margin: 30px 0 0 0;text-align: center; width: 100%;">
      <div class="buttons-wrapper" style=" display: inline-block; margin: 0 auto; width: 90%;">
        <a href="../upload" target="_blank" style="background: url(../../../' . $path . 'img/icon-upload.gif) no-repeat center top;' . $style . '"><span style="' . $style2 . '">Upload New Data</span></a>
        <a href="../download" target="_blank" style="background: url(../../../' . $path . 'img/icon-download.gif) no-repeat center top;' . $style . '"><span style="' . $style2 . '">Download Data</span></a>
        <a href="../" target="_blank" style="background: url(../../../' . $path . 'img/icon-rawdata.gif) no-repeat center top;' . $style . '"><span style="' . $style2 . '">Data Summary</span></a>
        <a href="../instructions" target="_blank" style="background: url(../../../' . $path . 'img/icon-instructions.gif) no-repeat center top;' . $style . '"><span style="' . $style2 . '">Standard Procedure</span></a>
        <div class="clear-float"></div>
      </div>
    </div></div>';

  return $output;
}


/**
 * Generate a readable and unique filename for the file to be generated.
 */
function rawpheno_trpdownload_get_filename($vars) {
  $filename = 'rawpheno_csv' . date('YMd') .'_'. time();
  return $filename;
}


/**
 * Determine the file suffix for the file to be generated.
 */
function rawpheno_trpdownload_get_suffix($vars) {
  return 'csv';
}

/**
 * Function callback: determine the human-readable file format.
 */
function rawpheno_trpdownload_get_readable_format($vars) {
  return 'Comma-separated Values';
}

/**
 * Function callback: generate csv file.
 */
function rawpheno_trpdownload_generate_file($variables, $job_id = NULL) {
  // Get query string and filename.
  $code = '';
  foreach($variables as $l => $v) {
    if(is_array($v)) {
      foreach($v as $j => $m) {
        if ($j == 'code') {
          $code = $m;
        }
      }
    }

    if ($l == 'filename') {
      $filename = $v;
    }
  }

  $q = base64_decode($code);
  list($project, $location, $traits, $r_version,,) = explode('&', $q);

  // Projects:
  $tmp = trim(str_replace('p=', '', $project));
  $project = explode(',', $tmp);

  // Locations:
  $location = trim(str_replace('l=', '', $location));

  if ($location == '0') {
    // Load all locations available.
    $sql = "SELECT DISTINCT value AS location
            FROM pheno_plantprop
            WHERE type_id = (SELECT cvterm_id FROM {cvterm} cvt LEFT JOIN {cv} cv ON cv.cv_id=cvt.cv_id WHERE cvt.name = 'Location' AND cv.name = 'phenotype_plant_property_types')
            ORDER BY value ASC";
    $location = chado_query($sql)
      ->fetchCol(0);
  }
  else {
    $location = explode(',', $location);
  }

  // Traits
  $traits = trim(str_replace('t=', '', $traits));

  if ($traits == 0) {
    // Load all traits available.
    $t = rawpheno_download_load_traits($location, $project);
    $traits = array_keys($t);
  }
  else {
    $traits = explode(',', $traits);
  }

  // Rversion
  $r_version = trim(str_replace('r=', '', $r_version));

  // Add planting date and # of seeds planted to the list of traits.
  // But, before doing this, make sure that these two column headers
  // are part or the project in the first place.
  $arr_notso_trait = array('Planting Date (date)', '# of Seeds Planted (count)');
  foreach($arr_notso_trait as $nt) {
    $sql = "SELECT cvterm_id FROM {cvterm} RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
            WHERE name = :cvterm AND project_id IN (:project_id) LIMIT 1";

    $n = chado_query($sql, array(':cvterm' => $nt, ':project_id' => $project));

    if ($n->rowCount() == 1) {
      $traits[] = $n->fetchField();
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

  $result = db_query($sql, $arr_q_string);

  // Array to hold column headers.
  //Add Name/Stock name column headers array.
  $header = array('A0' => 'Name');

  foreach ($result as $r) {
    $def = $r->name;

    // Get the R Friendly version when user requests for it.
    if ($r_version == 1) {
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

          SELECT t1.plant_id AS id, t1.type_id as tid, t2.name AS def, ARRAY_TO_STRING(ARRAY_AGG(DISTINCT t1.value), '<m_value>') AS value, 'C' as grp
          FROM {pheno_measurements} AS t1
          INNER JOIN chado.cvterm AS t2 ON t1.type_id = t2.cvterm_id
          WHERE t2.cvterm_id IN (:traits) AND t1.plant_id IN" . $sub_sql

          // Lastly we order the results by plant_id and grouping string, and tid.
          . "GROUP BY t1.plant_id, t1.type_id, t2.name
             ORDER BY id, grp, tid ASC";

  $results = db_query($sql, $arr_q_string);

  if ($results) {
    // Directories:
    $dir = array(
      'tripal_download' => variable_get('trpdownload_fullpath'),
      'tripal_temp'     => variable_get('file_temporary_path'),
      'tripal_public'   => drupal_realpath('public://'),
    );

    $filepath = $dir['tripal_download'] . $filename;

    drush_print("Generating CSV File: " . $filepath);
    $FILE = fopen($filepath, 'w') or die ('Uable to create file to write to');
    fputcsv($FILE, $header);

    // Build a multi-dimensional array with all the data in it. This array is keyed
    // 1st by plant_id and then by Group||cvterm_id (ie: B4243).
    // Note: this first array will not have any missing data cells filled in.
    $rows = array();
    foreach($results as $r) {
      $rows[ $r->id ][ $r->grp . $r->tid ] = $r->value;
    }

    // Total lines;
    $total_lines = count($rows);
    // INFO:
    // drush_print('Total Lines: ' . $total_lines);

    // Now we want to process each row to add entries for missing data.
    // We will do this by looping through the header for each row and if there isn't
    // already data set for that cell then we will add missing data.
    // Note: We will save the data to the file as we go.
    $cur_line = 0;
    $past_percent = '';
    drush_print('0% complete...');

    foreach ($rows as $row) {
      // To file... to screen...
      $percent = round(($cur_line / $total_lines) * 100);
      if ($percent % 10 == 0 && $past_percent != $percent) {
        drush_print($percent . '% complete...');
      }

      db_query('UPDATE {tripal_jobs} SET progress=:percent WHERE job_id=:id',
        array(':percent' => $percent, ':id' => $job_id));

      // First, add missing data for each entry in the header that is not in the row.
      foreach ($header as $id => $title) {
        if (!isset($row[$id])) {
          $row[$id] = 'NA';
        }
        else {
          $l = explode('<m_value>', $row[$id]);

          if (count($l) >= 2) {
            $row[$id] = str_replace('<m_value>',',', $row[$id]);
            // INFO:
            // Multiple values detected.
            // drush_print('Multiple values found: Line #' . ($cur_line + 1) . ' Col: ' . $title);
          }
        }
      }

      // Sort the row based on the keys to make sure they match with the header.
      ksort($row);

      // Write this row to the file as CSV.
      fputcsv($FILE, $row);

      $cur_line++;
      $past_percent = $percent;
    }

    // Finally, close the file.
    fclose($FILE);

    if ($percent != 100) {
      drush_print('100% complete...');
    }

    drush_print('Download complete');
  }
}
