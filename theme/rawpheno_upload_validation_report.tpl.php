<?php
/**
 * @file
 * Master template file of rawpheno upload file validation result message box.
 *
 * * Available variables:
 * - $status: An array of string status messages of file upload. see hook_validate() of upload page.
 * - $validators: Information about the validators available.
 */

// Get information about the validators available in oder to theme the validation results.
$validators = module_invoke_all('rawpheno_validators');
?>
<div class="window-info">
  <div id="window-validation-result">
    <h3>Validation Result</h3>
    <ul class="error-main">
      <?php 
        foreach ($status as $key => $result) {
          $class = ($result === TRUE) ? 'success' : 'failed';
          print '<li class="' . $class . '"><em>' . $validators[$key]['label'] . '</em></li>';
  
          // Provide further information if the validation failed.
          if ($result !== TRUE) {
            if (isset($validators[$key]['message callback']) AND function_exists($validators[$key]['message callback'])) {
              $messages = call_user_func($validators[$key]['message callback'], $result);
              if (!empty($messages)) {
                print '<ul class="error-detail"><li class="error-item">' . implode('</li><li class="error-item">', $messages) . '</li></ul>';
              }
            }
          }
        }
      ?>  
    </ul>
  </div>
  <br />
</div>
<hr class="button-collapse-infowindow window-on" />