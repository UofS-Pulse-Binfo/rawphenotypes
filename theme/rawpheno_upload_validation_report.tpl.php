<?php
/**
 *
 */

$messages = array(
  'is_xlsx' => 'File format is XLSX',
  'tab_exists' => '"Measurements" tab exists and has data',
  'missing_header' => '"Plot", "Entry", "Name", "Rep" and "Location" columns exists.',
);
?>

<h3>Validation Results</h3>
<ul>
<?php foreach ($status as $key => $result) {
  $class = ($result == TRUE) ? 'success' : 'failed';
  print '<li class="' . $class . '">' . $messages[$key] . '</li>';
  
  // Provide further information if the validation failed.
  if ($result !== TRUE) {
    if ($key == 'missing_header') {
      print '<ul><li>The following required columns are missing: "' . implode('", "', $result) . '".</li></ul>';
    }
  }
}?>
</ul>