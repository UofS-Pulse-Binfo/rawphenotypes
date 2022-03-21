<?php

/**
 * @file
 * Master template file of rawpheno germplasm (raw data) field.
 *
 * Available variables in $variables array (by array key):
 * - element_id    : value/id is used as the id attribute of the main container element. 
 *     All elments inside this container can be referenced using this id (for css or script).
 * - summary_table : is an associative array containing markup for summary table and header information.
 *   - table : themed table element, lists all traits, location, experiment and button to export data.
 *   - headers : header information - germplasm name, summary count of traits, location and experiment.
 *     - experiments : number of experiment.
 *     - locations   : number of location.
 *     - germplasm   : name of germplasm.
 *     - traits      : number of traits.
 */

// All theme table variables values are accessible throught this var.
$table =  $variables['summary_table'];
$table_header = $table['headers'];
?>

<div id="<?php print $variables['element_id']; ?>">
  <div id="rawphenotypes-germplasm-field-header">
    <div>
      <img src="<?php print $GLOBALS['base_url'] . '/' . drupal_get_path('module', 'rawpheno') . '/includes/TripalFields/ncit__raw_data/theme/icon-raw.jpg'; ?>" align="absmiddle"> &nbsp;&nbsp;<a href="#">What are Raw Phenotypes?</a>
    </div>
    
    <div>
      <div class="rawphenotypes-germplasm-nav">
        <a href="/phenotypes/raw/download">Go To Raw Phenotypes Download Page &#10095;</a>
      </div>
    </div>
    <div>&nbsp;</div>
  </div>
      
  <div id="rawphenotypes-define-raw-container" style="display: none;">
    Raw Phenotypes are raw data, not computed or averaged values and have not been published. 
    &nbsp;<a href="#">Okay, Got it!</a>
  </div>

  <div>
    <h1>
      <?php print $table_header['germplasm']; ?>: 
      <span>
        <?php print $table_header['traits']; ?> Traits / 
        <?php print $table_header['experiments']; ?> Experiments / 
        <?php print $table_header['locations']; ?> Locations
      </span>
    </h1>
  </div>
        
  <div id="rawphenotypes-germplasm-warning" class="messages warning">
    Please note that <i>some experiments</i> appear disabled. Please contact KnowPulse if you need access.
  </div>

  <div id="rawphenotypes-germplasm-table-wrapper">
    <div id="rawphenotypes-germplasm-export-table">
      <div><?php print $table['table']; ?></div>
    </div>
  </div>

  <div>
    <small>*Data export will launch a new window</small>
  </div>
  
  <div>&nbsp;</div>
</div>