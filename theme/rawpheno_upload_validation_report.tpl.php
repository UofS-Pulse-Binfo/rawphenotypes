<?php
/**
 *
 */

// Get information about the validators available in oder to theme the validation results.
$validators = module_invoke_all('rawpheno_validators');
?>

<h3>Validation Results</h3>
<ul>
<?php foreach ($status as $key => $result) {
  $class = ($result == TRUE) ? 'success' : 'failed';
  print '<li class="' . $class . '">' . $validators[$key]['label'] . '</li>';
  
  // Provide further information if the validation failed.
  if ($result !== TRUE) {
    if (isset($validators[$key]['message callback']) AND function_exists($validators[$key]['message callback'])) {
      $messages = call_user_func($validators[$key]['message callback'], $result);
      if (!empty($messages)) {
        print '<ul><li>' . implode('</li><li>', $messages) . '</li></ul>';
      }
    }
  }
}?>
</ul>