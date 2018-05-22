<?php

/**
 * Page callback: display standard phenotyping instructions page.
 */
function rawpheno_instructions($form, &$form_state, $project_id = NULL) {
   $tmp_project = $project_id;

  // Navigation button. Related page of instructions page is upload page.
  $form['page_button'] = array(
    '#type' => 'markup',
    '#markup' => t('Upload Data &#10095;'),
  );

  // Attach CSS.
  $path = drupal_get_path('module', 'rawpheno') . '/theme/';
  $form['#attached']['css'] = array($path . 'css/rawpheno.instructions.style.css');

  // Create a select box containing projects available - these are projects
  // that have associated column header set and must have at least 1 essential column header.
  // The projects are filtered to show only projects assigned to user.
  $all_project = rawpheno_function_user_project($GLOBALS['user']->uid);

  // No projects assined to the user.
  if (count($all_project) < 1) {
    return $form;
  }

  // Ensure that project is valid.
  if ($project_id) {
    if (!in_array($project_id, array_keys($all_project))) {
      // Project does not exist.
      $form['message_invalid_project'] = array(
        '#markup' => '<div class="messages error">Project does not exist.</div>'
      );
    }
    else {
      // Project is valid. Null this.
      $form['message_invalid_project'] = array();
    }
  }
  else {
    // When no project is supplied, then default this page to the most recent project
    // available from the projects defined by admin.
    $project_id = array_keys($all_project)[0];
  }

  // Project Name.
  $project_name = $all_project[$project_id];

  // Given a project id, construct the table required for each trait type set.
  // All trait types, need to remove type plantproperty.
  $trait_set = rawpheno_function_trait_types();

  $sql = "SELECT project_cvterm_id AS id FROM {pheno_project_cvterm}
          WHERE project_id = :project_id AND type <> :plantprop
          ORDER BY type, cvterm_id ASC";

  $args = array(':project_id' => $project_id, ':plantprop' => $trait_set['type4']);
  $cvterm_id = db_query($sql, $args);

  // Array to hold trait row.
  $arr_cvterm = array();

  foreach($cvterm_id as $id) {
    $cvterm = rawpheno_function_header_properties($id->id);
    // Create an array that will translate to a row in table.
    $arr_cvterm[ $cvterm['type'] ][] = array(
      '<div class="data-cells">' . $cvterm['name'] . '</div>',
      $cvterm['method'],
      $cvterm['definition']
    );
  }

  // Construct table.
  $arr_tbl_args['empty'] = '0 Column Header';
  $arr_tbl_args['header'] = array(t('Column Header/Trait'), t('Collection Method'), t('Definition'));

  unset($trait_set['type4']);

  foreach($trait_set as $type) {
    $arr_tbl_args['rows'] = isset($arr_cvterm[$type]) ? $arr_cvterm[$type] : array();

    $form['tbl_project_headers_' . $type] = array(
      '#markup' => (count($arr_tbl_args['rows']) <= 0) ? 'no-trait' : theme('table', $arr_tbl_args),
    );
  }

  // Project and project select box.
  $form['project_panel'] = array(
    '#markup' => $project_name
  );

  $form['sel_project'] = array(
    '#type' => 'select',
    '#options' => array(0 => 'Please select a project') + $all_project,
    '#id' => 'rawpheno-ins-sel-project'
  );

  $ins_path = base_path() . 'phenotypes/raw/instructions/';
  // Make the project select box into a jump menu and add the project id number to the url.
  drupal_add_js('jQuery(document).ready(function() {
    jQuery("#rawpheno-ins-sel-project").change(function(){
      if (jQuery(this).val() > 0) {
        window.location.href = "'. $ins_path .'" + jQuery(this).val();
      }
    });
  })', 'inline');


  // NOTE: When project is AGILE, download data collection spreadsheet link points to
  //       pre-made AGILE spreadsheet, otherwise, instructions page will generate the spreadsheet file. To ensure that
  //       the column headers matched for AGILE project, a check (when no new column header added tru admin then download
  //       pre-made, else generate) is required before downloading the file.

  // Provide user with link to download project specific data collection spreadsheet file.
  // When the project is the default project (AGILE) then supply a ready made spreadsheet.

  $file_path = ($tmp_project == null) ? 'instructions/spreadsheet/' : 'spreadsheet/';
  $link_to_xls = $file_path . $project_id;

  if ($project_name == 'AGILE: Application of Genomic Innovation in the Lentil Economy') {
    // Compare the AGILE-specific column header to check if they match
    $AGILE_column_headers = rawpheno_function_headers('expected');
    $AGILE_essential_headers = rawpheno_function_headers('essential');
    $AGILE_required_headers = rawpheno_function_headers('required');
    $AGILE_essential_headers = array_diff($AGILE_essential_headers, $AGILE_required_headers);

    // Query the AGILE project column headers.
    $sql = "SELECT name, type FROM {cvterm} RIGHT JOIN pheno_project_cvterm USING(cvterm_id) WHERE project_id = :project_id";
    $args = array(':project_id' => $project_id);
    $h = chado_query($sql, $args)
      ->fetchAllKeyed();

    // Manually add Name, since Name is not added to project.
    $h['Name'] = 'plantproperty';

    $header_found_count = 0;
    $essential_found_count = 0;
    $essential_count = 0;
    $header_count = 0;

    foreach($h as $header => $type) {
      // Ignore trait contributed.
      if ($type == $trait_set['type5']) {
        continue;
      }

      $header_count++;

      // Use ready made AGILE data collection spreadsheet.
      // Total number of headers, excluding contributed matches AGILE traits.
      if (in_array($header, $AGILE_column_headers)) {
        $header_found_count++;
      }

      // Same essential traits as in AGILE traits.
      if ($type == $trait_set['type1'] AND in_array($header, $AGILE_essential_headers)) {
        $essential_found_count++;
      }

      // Same number of essential traits as in AGILE traits.
      if ($type == $trait_set['type1']) {
        $essential_count++;
      }

      // Finally, name must be AGILE: Application of Genomic Innovation in the Lentil Economy as condition for this block.
    }

    // Check if AGILE trait set was not altered.
    if ($header_count == count($AGILE_column_headers)
        && $header_found_count == count($AGILE_column_headers)
        && $essential_found_count == count($AGILE_essential_headers)
        && $essential_count == count($AGILE_essential_headers)) {
      // AGILE Project match the original trait set. Download ready-made data collection spreadsheet file.
     $link_to_xls = file_create_url('public://AGILE-PhenotypeDataCollection-v5.xlsx.zip');
    }
  }

  $form['download_data_collection'] = array(
    '#markup' => t('<a href="@link" target="_blank">Download Data Collection Spreadsheet</a>', array('@link' => $link_to_xls))
  );

  // Search field with autocomplete feature.
  $form['txt_search'] = array(
    '#title' => '',
    '#type' => 'textfield',
    '#maxlength' => 65,
    '#size' => 65,
    '#default_value' => t('Search Trait'),
    '#autocomplete_path' => 'phenotypes/raw/instructions/autocomplete/' . $project_id,
  );

  $form['btn_search'] = array(
    '#type' => 'markup',
    '#markup' => '<input type="button" id="btn_submit" name="btn_search" class="form-submit" value=" ">',
  );

  // Hidden field containing url to json.
  $form['json_url'] = array(
    '#type' => 'hidden',
    '#value' => $GLOBALS['base_url'] . '/phenotypes/raw/instructions/autocomplete/' . $project_id,
    '#attributes' => array('id' => array('traits-json'))
  );

  // Attach JQuery UI library and JavaScript.
  $form['#attached']['library'][] = array('system', 'ui.tabs');
  $form['#attached']['js']  = array($path . 'js/rawpheno.instructions.script.js');

  return $form;
}


/**
 * Function create a spreadsheet file.
 *
 */
function rawpheno_instructions_create_spreadsheet($project_id) {
  // Query column headers specific to a project, given a project id.
  if (isset($project_id) AND $project_id > 0) {
    // Array to hold all trait types.
    $trait_type = rawpheno_function_trait_types();

    // Exclude the plant property from the set of headers. They are pre-inserted to the array
    // of column headers passed to the spreadsheet writer.
    $sql = "SELECT project_cvterm_id, name, type
            FROM {cvterm} RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
            WHERE project_id = :project_id AND type NOT IN ( :exclude_property )
            ORDER BY type ASC";

    $args = array(':project_id' => $project_id, ':exclude_property' => array($trait_type['type4'], $trait_type['type5']));
    $cvterm = chado_query($sql, $args);

    // Only when project has headers.
    if ($cvterm->rowCount() > 0) {
      // Array to hold the column headers passed to the excel writer.
      $col_headers = array();
      // Array to hold standard procedure, which basically is
      // the traits definition and collection method.
      $instructions_data = array();

      // Get the data type per unit. This type will be the cell type in the spreadsheet.
      $data_type = rawpheno_function_default_unit('type');

      // Prepend the array with plant property column headers.
      $col_headers = array(
        'Plot'     => 'integer',
        'Entry'    => 'integer',
        'Name'     => 'string',
        'Rep'      => 'integer',
        'Location' => 'string'
      );

      // Start at F column taking into account plant properties.
      // A for Plot, B for Entry and so on (A-E is 5 cols).
      $l = 'F';
      $cell_i = array();

      // Assign the data type for each header based on the unit it contains.
      $h = array('name' => 'Trait', 'definition' => 'Definition', 'method' => 'Collection Method');

      foreach($cvterm as $trait) {
        // Get the unit.
        $u = rawpheno_function_header_unit($trait->name);
        $unit = isset($data_type[$u]) ? $data_type[$u] : 'string';

        $col_headers[$trait->name] = $unit;

        // Highlight the cells when it is essential trait.
        if ($trait->type == $trait_type['type1']) {
          array_push($cell_i, $l . '1');
          // Increment F column.
          $l++;
        }

        // Get header method and definition information.
        $t = rawpheno_function_header_properties($trait->project_cvterm_id);

        foreach($h as $m_i => $m) {
          $star = ($m_i == 'name') ? '*' : '';
          array_push($instructions_data, array($star . $m . ':', $t[$m_i]));
        }

        // Add extra new line.
        array_push($instructions_data, array('' , ''));
      }

      // Load spreadsheet writer library.
      $xlsx_writer = libraries_load('spreadsheet_writer');
      include_once $xlsx_writer['library path'] . '/'. $xlsx_writer['files'][0];

      $writer = new XLSXWriter();
      // Measurement tab.
      @$writer->writeSheet(array(), 'Measurements', $col_headers,
        array(
          // The entire header row apply these styles.
          array(
            'font' =>
              array(
                'name'        => 'Arial',
                'size'        => '11',
                'color'       => '000000',
                'bold'        => false,
                'italic'      => false,
                'underline'   => false
            ),
            'wrapText'        => true,
            'verticalAlign'   => 'top',
            'horizontalAlign' => 'center',
            'fill'            => array('color' => 'F7F7F7'),
            'border'          => array('style' => 'thin', 'color' => 'A0A0A0'),
            'rows'            => array('0')
          ),
          // Once the styles above have been applied, style the plant property headers.
          array(
            'font' =>
              array(
                'name'        => 'Arial',
                'size'        => '11',
                'color'       => '000000',
                'bold'        => true,
                'italic'      => false,
                'underline'   => false
            ),
            'verticalAlign'   => 'bottom',
            'horizontalAlign' => 'center',
            'fill'            => array('color' => 'EAEAEA'),
            'border'          => array('style' => 'thin', 'color' => 'A0A0A0'),
            'cells'           => array('A1', 'B1', 'C1', 'D1', 'E1')
          ),
          // Make sure to style the essential trait/header.
          array(
            'font' =>
              array(
                'name'        => 'Arial',
                'size'        => '11',
                'color'       => '008000',
                'bold'        => true,
                'italic'      => false,
                'underline'   => false
            ),
            'wrapText'        => true,
            'verticalAlign'   => 'top',
            'horizontalAlign' => 'center',
            'fill'            => array('color' => 'F5FFDF'),
            'border'          => array('style' => 'thin', 'color' => 'A0A0A0'),
            'cells'           => $cell_i
          )
        )
      );

      // Standard procedure tab.
      // Load trait definition and data collection method to this sheet.
      $instructions_header = array();
      @$writer->writeSheet($instructions_data, 'Instructions', $instructions_header,
        array(
          array(
            'font' =>
              array(
                'name'        => 'Arial',
                'size'        => '11',
                'color'       => '000000',
                'bold'        => true,
                'italic'      => false,
                'underline'   => false
            ),
            'wrapText'        => true,
            'columns'         => '0',
          ),
          array(
            'font' =>
              array(
                'size'        => '12',
              ),
              'wrapText'      => false,
              'columns'       => '1',
          ),
        )
      );

      // Calculator tab.
      $calc_header = array('CALCULATE DAYS TO' => 'string');
      $calc_data =
        array(
          array('Planting Date', '2015-10-06'),
          array('Current Date', date('Y-m-d')),
          array('Current "Days till"', '=B3 - B2'),
          array('',''),
          array('Instructions', ''),
          array('', ''),
          array('Fill out the planting date indicated in the measurements tab, as well as, the current date.', ''),
          array('The "Days till" date will then be calculated for you.', '')
        );

      @$writer->writeSheet($calc_data, 'Calculate Days to', $calc_header,
        array(
          array(
            'font' =>
              array(
                'name'      => 'Arial',
                'size'      => '20',
                'color'     => '000000',
                'bold'      => true,
                'italic'    => false,
                'underline' => false
              ),
              'wrapText'    => false,
              'rows'        => array('0'),
          ),
          array(
            'font' =>
              array(
                'name'      => 'Arial',
                'size'      => '11',
                'color'     => 'FFFFFF',
                'bold'      => true,
                'italic'    => false,
                'underline' => false
              ),
              'wrapText'    => true,
              'fill'        => array('color' => '305673'),
              'border'      => array('style' => 'thin', 'color' => 'A0A0A0'),
              'rows'        => array('1'),
          ),
          array(
            'font' =>
              array(
                'name'      => 'Arial',
                'size'      => '11',
                'color'     => '000000',
                'bold'      => true,
                'italic'    => false,
                'underline' => false
              ),
              'wrapText'    => true,
              'fill'        => array('color' => 'F7F7F7'),
              'border'      => array('style' => 'thin', 'color' => 'A0A0A0'),
              'rows'        => array('2'),
          ),
          array(
            'font' =>
              array(
                'name'      => 'Arial',
                'size'      => '11',
                'color'     => '000000',
                'bold'      => true,
                'italic'    => false,
                'underline' => false
              ),
              'wrapText'    => true,
              'fill'        => array('color' => '79a183'),
              'border'      => array('style' => 'thin', 'color' => 'A0A0A0'),
              'rows'        => array('3'),
          )
        )
      );

      // Data collection spreadsheet name contains the following:
      // Project id, name of the user, date and time.
      $filename = 'datacollection_' . $project_id . '_' . str_replace(' ', '_', $GLOBALS['user']->name) .'_'. date('YMd') .'_'. time() . '.xlsx';
      $file = file_save_data($writer->writeToString(), 'public://' . $filename);

      // Launch save file window and ask user to save file.
      $http_headers = array(
        'Content-Type' => 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet',
        'Content-Disposition' => 'attachment; filename="' . $filename . '"',
      );

      if (strpos($_SERVER['HTTP_USER_AGENT'], 'MSIE')) {
        $http_headers['Cache-Control'] = 'must-revalidate, post-check=0, pre-check=0';
        $http_headers['Pragma'] = 'public';
      }
      else {
        $http_headers['Pragma'] = 'no-cache';
      }

      file_transfer($file->uri, $http_headers);

      // Just in case the auto download fails, provide a link to manually download the file.
      print '<a href="' . $file->uri . '">Download Data Collection Spreadsheet</a>';
    }
    else {
      // Here project not found.
      print 'Project not found.';
    }
  }
  else {
    // Project id is not valid.
    print 'Project ID number is invalid.';
  }
}
