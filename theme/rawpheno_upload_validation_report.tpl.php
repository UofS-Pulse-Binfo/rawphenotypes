<?php
/**
 *
 */

// Get information about the validators available in oder to theme the validation results.
$validators = module_invoke_all('rawpheno_validators');
?>
<h3>Validation Result</h3>
<ul class="error-main">
  <li class="success"><em>test</em><ul class="error-detail"><li class="error-item">Hello</li></ul></li>
  <li class="failed"><em>test</em><ul class="error-detail"><li class="error-item">Hello</li></ul></li>
  
  <?php 
    foreach ($status as $key => $result) {
      $class = ($result == TRUE) ? 'success' : 'failed';
      print '<li class="' . $class . '"><em>' . $validators[$key]['label'] . '</em></li>';
  
      // Provide further information if the validation failed.
      if ($result !== TRUE) {
        if (isset($validators[$key]['message callback']) AND function_exists($validators[$key]['message callback'])) {
          $messages = call_user_func($validators[$key]['message callback'], $result);
          if (!empty($messages)) {
             print '<ul class="error-detail"><li class="error-item">' . implode('</li><li>', $messages) . '</li></ul>';
          }
        }
      }
    }
  ?>  
</ul>