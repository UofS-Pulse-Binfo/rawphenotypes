<?php
/**
 * @file
 * Functions providing validation of raw phenotypic data.
 * These functions are currently used when new data is being uploaded.
 */

/**
 * Implements hook_rawpheno_validators().
 *
 * This hook is used to tell the rawpheno module about any validation you want done on
 * the file being uploaded. It's a hook to allow custom modules to add organization-specific
 * validation as needed.
 *
 * This module provides the following validation:
 *   - ensures required information columns have a value in all cells.
 *   - ensures the value matches the unit indicated.
 *   - ensures the germplasm already exists in the stock table.
 *   - ensures that specific traits are always present in the header (allows missing data).
 *
 * @return
 *   An array describing different validators that should be used. Each item in the array
 *   is a validator definition where the key is the unique, machine name of the validator.
 *   Each validator defition should have the following keys.
 *     - label: Human-readable label used on the upload validation page.
 *     - scope: Whether this validation should be done on all rows ('all'), a random subset ('subset'), 
 *         or even just the header ('header').
 *     - headers: an array of specific columns to validate. If null then will validate all columns.
 *     - message callback: The name of a function used to generate additional error 
 *         messages to give the user more input on how to fix the error. If NULL then no 
 *         additional message will be given.
 *     - validation callback: The name of the function that should be called to validate 
 *         a single cell.
 */
function rawpheno_rawpheno_validators() {
  $validators = array();
  
  $validators['is_excel'] = array(
    'label' => 'Uploaded file is a Microsoft Excel Spreadsheet.',
    'scope' => 'file',
    'message callback' => NULL,
    'validation callback' => 'validator_is_excel_validate_file',
  );
  
  $validators['tab_exists'] = array(
    'label' => 'The "Measurements" tab exists and has content.',
    'scope' => 'file',
    'message callback' => NULL,
    'validation callback' => 'validator_tab_exists_validate_file',
  );

  // Ensure that specific traits are in the spreadsheet.
  $validators['column_exists'] = array(
    'label' => 'Essential traits are present.',
    'scope' => 'header',
    'message callback' => 'validator_column_exists_generate_msg',
    'validation callback' => 'validator_column_exists_validate_header',
  );
  
  // Ensure that required information is present for all cells in those columns.
  $validators['required_info'] = array(
    'label' => 'Required Information is present and numbers are not negative.',
    'scope' => 'subset',
    'headers' => array('Plot','Entry','Name','Rep','Location'),
    'message callback' => 'validator_required_info_generate_msg',
    'validation callback' => 'validator_required_info_validate_cell',
  );
  
  // Ensure that the type of data in a cell is consistent with the unit of measurement.
  $validators['units_match_type'] = array(
    'label' => 'Data was measured using expected units.',
    'scope' => 'subset',
    'message callback' => 'validator_units_match_type_generate_msg',
    'validation callback' => 'validator_units_match_type_validate_cell',
  );

  // Ensure that all germplasm already exists.
  $validators['germplasm_present'] = array(
    'label' => 'All Germplasm names are recognized by this resource.',
    'scope' => 'all',
    'headers' => array('Name'),
    'message callback' => 'validator_germplasm_present_generate_msg',
    'validation callback' => 'validator_germplasm_present_validate_cell',
  );
  
  return $validators;
}

/**
 * Check that the current file has an extension of xlsx or xls.
 *
 * @param $file
 *   A drupal managed_file object describing the uploaded spreadsheet.
 * @return
 * TRUE if the file is either xls or xlsx; FALSE otherwise.
 */
function validator_is_excel_validate_file($file) {

  // First test extension.
  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);
  if ($xls_extension == 'xlsx' OR $xls_extension == 'xls') {
  
    // Then test that the file can be opened.
    // This checks against PDFs masquerading as XLS ;-).
    try {
      $xls_obj = rawpheno_open_file($file);
    }
    catch (Exception $E) {
      return FALSE;
    }

    if ($xls_obj) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Check that there is a measurements tab.
 * @todo: check that tab has data?
 *
 * @param $file
 *   A drupal managed_file object describing the uploaded spreadsheet.
 * @return
 *   TRUE if the file has a measurements tab; FALSE otherwise.
 */
function validator_tab_exists_validate_file($file) {

  // Check it opens.
  $xls_obj = rawpheno_open_file($file);

  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_value) {
    $xls_obj->ChangeSheet($sheet_key);

    // check the measurements tab exists.
    if (rawpheno_function_delformat($sheet_value) == 'measurements') {
    
      // Check there are contents.
      $i = 0;
      $have_data = 0;
      foreach ($xls_obj as $row) {
        // only check the first two lines.
        if ($i > 2) break;
        
        // Check the row isn't empty.
        if (strlen(trim(implode('', $row))) >= 1) {
          $have_data++;
        }
        $i++;
      }
      if ($have_data > 1) return TRUE;
    }
  }
  
  return FALSE;
}

/**
 * Provide more useful messages to the user for if validation failed.
 *
 * @param $error_info
 *   Whatever information was returned from the validation callback indexed first by
 *   column name and then by row number.
 * @return
 *   An array of human-readable messages to the user telling them exactly what failed validation.
 */
function validator_column_exists_generate_msg($error_info) {
  $messages = array();
  
  if (isset($error_info['missing'])) {
    $messages[] = 'The following columns must exist: "' . implode('", "',$error_info['missing']). '".';
  }
  
  if (isset($error_info['duplicates'])) {
    $messages[] = 'The following columns were duplicated: "' . implode('", "',$error_info['duplicates']). '". Please ensure each column is unique (only occurs once in the spreadsheet).';
  }
  
  return $messages;
}

/**
 * @TODO
 *
 * @param $header
 *   An array containing the header.
 *
 * @return
 *   TRUE if it passed validation; FALSE otherwise.
 */
function validator_column_exists_validate_header($header) {
  $result = array();

  // First check that all the essential traits are there.
  $delformat_header = array();
  $nospace_header = array();
  foreach ($header as $h) {
    $delformat_header[] = $h['no units'];
    $nospace_header[] = str_replace(' ','', $h['no format']);
  }

  $essential_traits = array(
    'Plot',
    'Entry',
    'Name',
    'Rep',
    'Location',
    'Planting Date',
    '# of Seeds Planted',
    'Days to Emergence',
    '# of Emerged Plants',
    'Days till 10% of Plants have Elongated Tendrils',
    'Days till 10% of Plants have One Open Flower',
    'Days till 10% of Plants have 1/2 Pods Mature',
    'Days till Harvest',
    'Diseases Present'
  );
  $missing_traits = array_diff($essential_traits, $delformat_header);
  
  if (!empty($missing_traits)) {
    $result['missing'] = $missing_traits;
  }

  // Second check there are no duplicates.
  $nospace_header = array_filter($nospace_header);
  $counts = array_count_values($nospace_header);
  // If there are duplicates then go ahead and find them.
  if (sizeof($nospace_header) != sizeof($counts)) {
    
    $duplicates = array();
    foreach($counts as $val => $c) {
      if($c > 1) {
        $key = array_search($val, $nospace_header);
        $duplicates[] = $header[$key]['no units'];
      }
    }
    $result['duplicates'] = $duplicates;
  }  

  if (empty($result)) {
    return TRUE;
  }
  else {
    return $result;
  }
}

/**
 * Provide more useful messages to the user for if validation failed.
 *
 * @param $error_info
 *   Whatever information was returned from the validation callback indexed first by
 *   column name and then by row number.
 * @return
 *   An array of human-readable messages to the user telling them exactly what failed validation.
 */
function validator_required_info_generate_msg($error_info) {
  $messages = array();
  
  // For each column with missing values.
  foreach ($error_info as $column => $rows) {
  
    // Find the parts and separate the messages.
    $empty_parts = array();
    $dup_parts = array();
    foreach ($rows as $row_index => $info) {
      if ($info['code'] == 1) {
        $empty_parts[] = $row_index;
      }
      elseif ($info['code'] == 2) {
        $dup_parts[ $info['value'] ] = '"' . $info['value'] . '" on lines ' . implode(', ', $info['rows']);
      }
    }

    // Generate the empty message.
    if (!empty($empty_parts)) {
      $msg = "You need to supply non-negative values for \"$column\" on row";
      
      // pluralize rows if there are more than one with an empty value.
      if (sizeof($empty_parts) > 1) { $msg .= 's'; }
    
      // Add the row numbers to the message.
      $msg .= ' ' . implode(', ',$empty_parts) . '.';
    
      $messages[] = $msg;
    
    }
    
    // Generate the duplicate values message.
    if (!empty($dup_parts)) {
      $placeholder = (sizeof($dup_parts) > 1) ? 'values are' : 'value is';
      $msg = "The \"$column\" should be unique. The following $placeholder duplicated: " 
        . implode('; ', $dup_parts) . '.';
      $messages[] = $msg;
    }

  }
  
  return $messages;
}

/**
 * Ensures that the current cell is not empty.
 *
 * @param $value
 *   The value of the current cell to be validated.
 * @param $context
 *   Information providing context to the cell to validate. Includes:
 *    - 'row index': the index of the tow the cell is in.
 *    - 'row': an array containing all cells in the current row.
 *    - 'column index': the index of the column the cell is in.
 *    - 'header': an array containing the header for the file. Each column in the header
 *        will be pre-processed with the following self-explanitory keys: 'no format', 
 *       'original', 'units', 'no units'.
 * @param $storage
 *   An empty variable that is specific to the current validation but persistent across
 *   rows. This can be helpful when checking that values are unique, etc.
 *
 * @return
 *   TRUE if it passed validation; FALSE otherwise.
 */
function validator_required_info_validate_cell($value, $context, &$storage) {

  // Validate if the cell is empty.
  if (empty($value)) return array('code' => 1, 'row' => $context['row index']);
  if ($value < 0) return array('code' => 1, 'row' => $context['row index']);
  
  // Validate if the plot is unique.
  // If this is the plot column...
  if ($context['header'][ $context['column index'] ]['no format'] == 'plot') {

    // Has this value been used before?
    $rows_used_in = array_keys($storage, $value);
    
    // save to storage.
    $storage[ $context['row index'] ] = $value;
    
    // if it has then save for error.
    if ($rows_used_in) {
      $rows_used_in[] = $context['row index'];
      return array('code' => 2, 'value' => $value, 'rows' => $rows_used_in);
    }
  }
  return TRUE;
}

/**
 * Provide more useful messages to the user for if validation failed.
 *
 * @param $error_info
 *   Whatever information was returned from the validation callback indexed first by
 *   column name and then by row number.
 * @return
 *   An array of human-readable messages to the user telling them exactly what failed validation.
 */
function validator_units_match_type_generate_msg($error_info) {
  $messages = array();
  
  foreach($error_info as $column_name => $rows) {

    // Expand each row.
    $parts = array();
    foreach ($rows as $row_index => $error) {
      $parts[] = '"' . $error['value'] .'" (line '.$row_index.')';
      $unit_name = $error['unit'];
    }
    
    // We want our messages consistent but helpful thus we need to provide some
    // customization to the message based on the unit.
    $unit_msg = '';
    switch($unit_name) {
      case 'date':
        $unit_name = 'date(s)';
        $unit_msg = 'We expect the date to be YYYY-MM-DD and to have occured in the past.';
        break;
      case 'count':
        $unit_name = 'count(s)';
        $unit_msg = 'We expect positive whole numbers.';
        break;
      case 'days':
        $unit_name = 'day(s)';
        $unit_msg = 'We expect positive whole numbers.';
        break;
      case 'cm':
        $unit_name = 'centimeters';
        break;
      case 'g':
        $unit_name = 'grams';
        break;
      case 'y/n/?':
        break;
      case 'scale':
        $unit_name = 'within the scale';
        $unit_msg . 'It should be a whole number between 1 and 5 where 1 indicates most plants are fully upright and 5 indicates most plants are fully prostrate/flat.';
        break;
      case 'text':
    }
    
    // Combine all the parts into a full message.
    $messages[] = "The following values in \"$column_name\" are not $unit_name: " . implode(', ', $parts) . '. ' . $unit_msg;
  }
  
  return $messages;
}

/**
 * Ensures that the type of data matches the unit.
 *
 * @param $value
 *   The value of the current cell to be validated.
 * @param $context
 *   Information providing context to the cell to validate. Includes:
 *    - 'row index': the index of the tow the cell is in.
 *    - 'row': an array containing all cells in the current row.
 *    - 'column index': the index of the column the cell is in.
 *    - 'header': an array containing the header for the file. Each column in the header
 *        will be pre-processed with the following self-explanitory keys: 'no format', 
 *       'original', 'units', 'no units'.
 * @param $storage
 *   An empty variable that is specific to the current validation but persistent across
 *   rows. This can be helpful when checking that values are unique, etc.
 *
 * @return
 *   TRUE if it passed validation; FALSE otherwise.
 */
function validator_units_match_type_validate_cell($value, $context, &$storage) { 

  // Determine the units for this column.
  $unit = $context['header'][ $context['column index'] ]['units'];

  // We always want to allow empty cells but not treat zero as empty...
  if (empty($value) AND !(strval($value) === '0')) {
    return TRUE;
  }
  
  // Then check if it matches based on the unit.
  switch($unit) {
    case 'date':
      // Date must be YYYY-MM-DD format.
      if (preg_match('/^([0-9]{4})-([0-9]{2})-([0-9]{2})/', $value, $matches)) {

        // Check that the month/day are in the expected range.
        $year = $matches[1];
        $month = $matches[2];
        $day = $matches[3];
        if ($month >= 1 AND $month <= 12 AND $day >= 1 AND $day <= 31 AND $year > 1900) {
        
          // Then check that it's not in the future.
          $today = new DateTime();
          $date = DateTime::createFromFormat('Y-m-d', $value);
          if ($date <= $today) {
            return TRUE;
          }
        }
      }
      break;
    
    case 'count':
    case 'days':
      // First check that it's a number.
      if (is_numeric($value)) {
        // Then check it's a positive whole number.
        if ( (int)$value == $value AND $value > 0) {
          return TRUE;
        }
      }
      // Allow 0 for count.
      if ($unit == 'count' AND strval($value) === '0') {
        return TRUE;
      }
      break;
      
    case 'cm':
    case 'g':
      // Measurements - if it is numeric, then it must be greater than or equals to 0.
      if (is_numeric($value) && $value > 0) {
        return TRUE;
      }
      // Allow 0
      if (strval($value) === '0') {
        return TRUE;
      }
      break;
    
    case 'y/n/?':
      // Yes or No - if it is char, length is one, then check if y, n, ? (question mark).
      if (in_array(strtolower($value), array('y','n','?', 'yes', 'no'))) {
        return TRUE;
      }
      break;
    
    case 'scale':
      // Scales are associated with the trait cvterm and each approved code is stored
      // in pheno_scale_member.
      // First retrieve the cvterm_id for the trait.
      $trait = $context['header'][ $context['column index'] ]['original'];
      $trait_id = rawpheno_get_trait_id($trait);
      if ($trait_id) {
        // Then check that the value is in the scale members
        $present = db_query('SELECT true FROM {pheno_scale_member} WHERE scale_id=:trait_id AND code=:value', 
          array(':trait_id' => $trait_id, ':value' => $value))->fetchField();
        if ($present) return TRUE;
        // If not, make sure there are members.
        $has_scale = db_query('SELECT count(*) FROM {pheno_scale_member} WHERE scale_id=:trait_id',
          array(':trait_id' => $trait_id))->fetchField();
        if (!$has_scale) return TRUE;
      }
      // If this is a new trait/column then we don't know how to validate it, so we assume it's correct.
      else {
        return TRUE;
      }
      break;
    case 'text':
      return TRUE;
      break;
  }

  return array('value' => $value, 'unit' => $unit);
}

/**
 * Provide more useful messages to the user for if validation failed.
 *
 * @param $error_info
 *   Whatever information was returned from the validation callback indexed first by
 *   column name and then by row number.
 * @return
 *   An array of human-readable messages to the user telling them exactly what failed validation.
 */
function validator_germplasm_present_generate_msg($error_info) {
  $nonexistant = array();
  $nonunique = array();
  
  foreach ($error_info['Name'] as $v) {
    if ($v['code'] === 0) {
      $nonexistant[] = '"' . $v['name'] . '" (line ' . $v['row number'] . ')';
    }
    elseif ($v['code'] === 9) {
      $nonunique[] = '"' . $v['name'] . '" (line ' . $v['row number'] . ')';
    }
  }
  
  $messages = array();
  if (!empty($nonexistant)) {
    $messages[] = 'The following germplasm does not already exist: '. implode(', ',$nonexistant).'.';
  }
  if (!empty($nonunique)) {
    $messages[] = 'The following germplasm names are not unique in this resource: '. implode(', ',$nonunique).'. Please notify the administrator of this duplication.';
  }
  
  return $messages;
}

/**
 * Ensures that the germplasm in the current cell exists and is unique.
 *
 * @param $value
 *   The value of the current cell to be validated.
 * @param $context
 *   Information providing context to the cell to validate. Includes:
 *    - 'row index': the index of the tow the cell is in.
 *    - 'row': an array containing all cells in the current row.
 *    - 'column index': the index of the column the cell is in.
 *    - 'header': an array containing the header for the file. Each column in the header
 *        will be pre-processed with the following self-explanitory keys: 'no format', 
 *       'original', 'units', 'no units'.
 * @param $storage
 *   An empty variable that is specific to the current validation but persistent across
 *   rows. This can be helpful when checking that values are unique, etc.
 *
 * @return
 *   TRUE if it passed validation. If it failed an array providing information about the
 *   failure is returned. Specifically, the name, row number, an error code, etc.
 *
 * Error Code:
 *   0 = no matching stock.
 *   9 = too many matching stocks (not unique).
 */
function validator_germplasm_present_validate_cell($value, $context, &$storage) {
  
  // We don't want this test to catch empty cells. There is a separate test for testing required values are resent.
  if (empty($value)) {
    return TRUE;
  }
  
  $stocks = chado_query('SELECT stock_id FROM {stock} WHERE name=:name', array(':name' => $value))->fetchAll();
  
  if (sizeof($stocks) == 1) {
    return TRUE;
  } 
  elseif (empty($stocks)) {
    return array('name' => $value, 'row number' => $context['row index'], 'code' => 0);
  }
  else {
    return array('name' => $value, 'row number' => $context['row index'], 'code' => 9, 'stocks' => $stocks);
  }

}
