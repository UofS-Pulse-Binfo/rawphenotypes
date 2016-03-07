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
    'label' => 'The "Measurements" tab exists and contains data.',
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
    'label' => 'Required Information is present.',
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

  $xls_extension = pathinfo($file->filename, PATHINFO_EXTENSION);
  if ($xls_extension == 'xlsx' OR $xls_extension == 'xls') {
    return TRUE;
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

  $xls_obj = rawpheno_open_file($file);

  $xls_sheets = $xls_obj->Sheets();
  foreach($xls_sheets as $sheet_key => $sheet_value) {
    $xls_obj->ChangeSheet($sheet_key);

    if (rawpheno_function_delformat($sheet_value) == 'measurements') {
      return TRUE;
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
  
  $messages[] = 'The following columns must exist: "' . implode('", "',$error_info). '".';
  
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

  $delformat_header = array();
  foreach ($header as $h) {
    $delformat_header[] = $h['no units'];
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
    return $missing_traits;
  }
  else {
    return TRUE;
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
  
    $msg = "You need to supply values for \"$column\" on row";
    
    // pluralize rows if there are more than one with an empty value.
    if (sizeof($rows) > 1) { $msg .= 's'; }
    
    // Add the row numbers to the message.
    $msg .= ' ' . implode(', ',$rows) . '.';
    
    $messages[] = $msg;
  }
  
  return $messages;
}

/**
 * Ensures that the current cell is not empty.
 *
 * @param $value
 *   The value of the current cell to be validated.
 * @param $row_index
 *   The index of the current row.
 * @param $row
 *   An array containing the current row -meant to provide context if needed.
 * @param $header
 *   An array containing the header -meant to provide context if needed.
 *
 * @return
 *   TRUE if it passed validation; FALSE otherwise.
 */
function validator_required_info_validate_cell($value, $row_index, $row, $header) { 
  return empty($value) ? FALSE : TRUE;
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
    $parts = array();
    foreach ($rows as $row_index => $error) {
      $msg = "The following values in \"$column_name\" are not " . $error['unit'] . ": ";
      $parts[] = '"' . $error['value'] .'" (line '.$row_index.')';
    }
    $messages[] = $msg . implode(', ', $parts) . '.';
  }
  
  return $messages;
}

/**
 * Ensures that the type of data matches the unit.
 *
 * @param $value
 *   The value of the current cell to be validated.
 * @param $row_index
 *   The index of the current row.
 * @param $row
 *   An array containing the current row -meant to provide context if needed.
 * @param $header
 *   An array containing the header -meant to provide context if needed.
 *
 * @return
 *   TRUE if it passed validation; FALSE otherwise.
 */
function validator_units_match_type_validate_cell($value, $row_index, $row, $column_index, $header) { 

  // Determine the units for this column.
  $unit = $header[$column_index]['units'];

  // We always want to allow empty cells.
  if (empty($value)) {
    return TRUE;
  }
  
  // Then check if it matches based on the unit.
  switch($unit) {
    case 'date':
      // Date must be YYYY-MM-DD format.
      if (preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}/', $value)) {
        return TRUE;
      }
      break;
    
    case 'count':
    case 'days':
    case 'cm':
    case 'g':
      // Measurements - if it is numeric, then it must be greater than or equals to 0.
      if (is_numeric($value) && $value >= 0) {
        return TRUE;
      }
      break;
    
    case 'y/n/?':
      // Yes or No - if it is char, length is one, then check if y, n, ? (question mark).
      if (strlen($value) == 1 && in_array($value, array('y','n','?'))) {
        return TRUE;
      }
      break;
    
    case 'scale':
      // Scale - if numeric, check if 1-5 range, inclusive.
      if (is_numeric($value) && ($value <= 6 || $value >= 1)) {
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
 * @param $row_index
 *   The index of the current row.
 * @param $row
 *   An array containing the current row -meant to provide context if needed.
 * @param $header
 *   An array containing the header -meant to provide context if needed.
 *
 * @return
 *   TRUE if it passed validation. If it failed an array providing information about the
 *   failure is returned. Specifically, the name, row number, an error code, etc.
 *
 * Error Code:
 *   0 = no matching stock.
 *   9 = too many matching stocks (not unique).
 */
function validator_germplasm_present_validate_cell($value, $row_index, $row, $header) {
  
  $stocks = chado_query('SELECT stock_id FROM {stock} WHERE name=:name', array(':name' => $value))->fetchAll();
  
  if (sizeof($stocks) == 1) {
    return TRUE;
  } 
  elseif (empty($stocks)) {
    return array('name' => $value, 'row number' => $row_index, 'code' => 0);
  }
  else {
    return array('name' => $value, 'row number' => $row_index, 'code' => 9, 'stocks' => $stocks);
  }

}
