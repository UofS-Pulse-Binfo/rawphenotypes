<?php
/**
 * @file
 * Master template file of rawpheno germplasm (raw data) field.
 *
 * * Available variables in $variables array (by array key):
 * - element_id - value/id used to in the id attribute of the main container element. All elments 
 *        inside this element can be referenced using this id.
 * - $validators: Information about the validators available.
 */
print $variables['raw_data'];
?>

<div id="<?php print $variables['element_id']; ?>">
  <div id="rawphenotypes-germplasm-field-header">
    <div>
      <img src="<?php print $variables['path_img'] . 'icon-raw.jpg'; ?>" align="absmiddle"> &nbsp;&nbsp;<a href="#">What are Raw Phenotypes?</a>
    </div>
    
    <div>
      <div class="rawphenotypes-germplasm-nav">
        <a href="/phenotypes/raw/download">Go To Download Page &#10095;</a>
      </div>
    </div>
    <div>&nbsp;</div>
  </div>
      
  <div id="rawphenotypes-define-raw-container" style="display: none;">
    Raw Phenotypes are raw data, not computed or averaged values and have not been published. &nbsp;<a href="#">Okay, Got it!</a>
  </div>

  <div>
    <h1>GERMPLASM NAME: <span><?php print $variables['header']['traits']; ?> Traits / <?php print $variables['header']['experiments']; ?> Experiments / <?php print $variables['header']['locations']; ?> Locations</span></h1>
  </div>
        
  <div id="rawphenotypes-germplasm-warning" class="messages warning">
    Please note that <i>some experiments</i> appear disabled. Please contact KnowPulse if you need access.
  </div>

  <div id="rawphenotypes-germplasm-table-wrapper">
    <div id="rawphenotypes-germplasm-export-table">
      <div><?php print $variables['summary_table']; ?></div>
    </div>
  </div>

  <div><small>*Data export will launch a new window</small></div>
</div>