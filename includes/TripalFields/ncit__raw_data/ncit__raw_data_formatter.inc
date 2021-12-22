<?php
/**
 * @class
 * Purpose: Provide a quick search on entity pages which submits/redirects to a full search.
 *
 * Display: A simple textfield search form.
 * Configuration:
 *   - path to the full search.
 *   - the URL token (query parameter) the value applies to.
 *   - help text.
 *   - textfield placeholder.
 *   - search button text.
 *   - autocomplete path.
 */
class ncit__raw_data_formatter extends TripalFieldFormatter {
  // The default label for this field.
  public static $default_label = 'Germplasm Raw Phenotypes';

  // The list of field types for which this formatter is appropriate.
  public static $field_types = array('ncit__raw_data');

  /**
   *  Provides the display for a field
   *
   * This function corresponds to the hook_field_formatter_view()
   * function of the Drupal Field API.
   *
   *  This function provides the display for a field when it is viewed on
   *  the web page.  The content returned by the formatter should only include
   *  what is present in the $items[$delta]['values] array. This way, the
   *  contents that are displayed on the page, via webservices and downloaded
   *  into a CSV file will always be identical.  The view need not show all
   *  of the data in the 'values' array.
   *
   *  @param $element
   *  @param $entity_type
   *  @param $entity
   *  @param $langcode
   *  @param $items
   *  @param $display
   *
   *  @return
   *    An element array compatible with that returned by the
   *    hook_field_formatter_view() function.
   */
  public function view(&$element, $entity_type, $entity, $langcode, $items, $display) {
    if ($items[0]['value']['NCIT:Raw Data']) {
      $germplasm_raw_phenotypes = $items[0]['value']['NCIT:Raw Data'];

      // Create markup.
      // Refer to this ID for CSS styling.
      $id = 'rawphenotypes-germplasm-raw-phenotypes-field';
      $markup = '
        <div id="' . $id . '">
        <div id="rawphenotypes-germplasm-field-header">
          <div>
            <img src="' . $germplasm_raw_phenotypes['icons']['raw'] . '" align="absmiddle"> &nbsp;&nbsp;<a href="#">What are Raw Phenotypes?</a>
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

        <div><h1>' . $germplasm_raw_phenotypes['germplasm']['name'] . ': <span>%d Traits / %d Experiments / %d Locations</span></h1></div>
        
        <div id="rawphenotypes-germplasm-warning" class="messages warning">Please note that <i>some experiments</i> appear disabled. Please contact KnowPulse if you need access.</div>

        <div id="rawphenotypes-germplasm-table-wrapper">
          <div id="rawphenotypes-germplasm-export-table">
            <div>%s</div>
          </div>
        </div>

        <div><small>*Data export will launch a new window</small></div>
      </div>';
      $icon_img = '<img id="rawphenotypes-germplasm-field-trait-select-%s-img" src="%s" border="0" align="absmiddle" alt="Download Raw Phenotypic Data" title="Download Raw Phenotypic Data" />';

      $response = '';
      
      if ($germplasm_raw_phenotypes['user']['permission']) {
        // Export summary table.  
        $table_row = array();  
        
        $id = 0;
        foreach($germplasm_raw_phenotypes['traits'] as $trait => $exp_loc) {
          list($trait_id, $trait_name) = explode('_', $trait);

          $select = $this->create_select($germplasm_raw_phenotypes['germplasm']['id'], $exp_loc, $germplasm_raw_phenotypes['user']['experiments']);
          $table_row[ $id ] = array(sprintf($icon_img, '', $germplasm_raw_phenotypes['icons']['leaf']) . ucfirst($trait_name), $select, sprintf($icon_img, $trait_id, $germplasm_raw_phenotypes['icons']['export'])); 
          $id++;
        }
       
        // Create markup.
        $summary_table = theme('table', array(
          'header' => array('Trait', 'LOCATION/Experiment', sprintf($icon_img, '', $germplasm_raw_phenotypes['icons']['download'])),
          'rows' => $table_row,
          'sticky' => FALSE,
          'attributes' => array('id' => 'rawphenotypes-germplasm-field-table')
        ));

        $response = sprintf($markup, 
          $germplasm_raw_phenotypes['summary']['traits'], 
          $germplasm_raw_phenotypes['summary']['experiments'], 
          $germplasm_raw_phenotypes['summary']['locations'], 
          $summary_table
        );

        // Render germplasm raw phenotypes.
        $element[0] = array(
          '#type' => 'markup',
          '#markup' => $response,
        );
      }  
    }

    return $element;
  }
  
  /**
   * Create select field.
   * 
   * @param $germplasm
   *   Stock id number.
   * @param $items
   *   Associative array, where each item will be rendered as an option
   *   with key as the value and value as text.
   * @param $disable
   *   Array of items to match an item if it should be disabled.
   */
  public function create_select($germplasm, $items, $disable) {
    $option = array();
    $cache_exp = [];
    foreach($items as $loc_exp) {
      list($trait_id, $project_id, $project_name, $location) = explode('#', $loc_exp);
      $select_value = $trait_id . '#' . $project_id . '#' . $location . '#' . $germplasm;
      $cache_exp[] = $project_id;

      $disabled = (in_array($project_id, $disable)) ? '' : 'disabled';
      $option[] = '<option value ="' . $select_value . '" ' . $disabled . '>' . strtoupper($location) . '/' . $project_name . '</option>';
    }
  
    $select = '<select class="form-select" id="rawphenotypes-germplasm-field-trait-select-' . $trait_id . '">
    <option value="0" selected>Select (' . count($items) . ' Locations, ' . count(array_unique($cache_exp)) . ' Experiments)</option>
    %s
    </select>';

    return sprintf($select, implode('', $option));
  }
}

