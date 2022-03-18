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
    if ($items[0]['value'] && $items[0]['value']['user']['permission']) {
      $path_base = $GLOBALS['base_url'] . '/'; 
      $path_module = drupal_get_path('module', 'rawpheno') . '/';
      
      // Add style and script.
      drupal_add_css($path_module . 'theme/css/rawpheno.germplasmfield.style.css');  
      drupal_add_js($path_base . $path_module . 'theme/js/rawpheno.germplasmfield.script.js');

      $germplasm_raw_phenotypes = $items[0]['value'];

      // Export summary table.  
      $table_row = array();  

      $id = 0;
      foreach($germplasm_raw_phenotypes['traits'] as $trait => $exp_loc) {
        list($trait_id, $trait_name) = explode('_', $trait);

        $select = $this->create_select($exp_loc, $germplasm_raw_phenotypes['user']['experiments']);
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
      
      // Render germplasm raw phenotypes.
      $variables = array(
        'element_id' =>  'rawphenotypes-germplasm-raw-phenotypes-field',
        'path_img' => $path_base . $path_module . 'theme/img/fields/',
        'summary_table' => $summary_table,
        'header' => array(
          'traits' => $germplasm_raw_phenotypes['summary']['traits'], 
          'experiments' => $germplasm_raw_phenotypes['summary']['experiments'], 
          'locations' => $germplasm_raw_phenotypes['summary']['locations'],
        )       
      );

      $markup = theme('rawpheno_germplasm_field', $variables);
      $element[0] = array(
        '#type' => 'markup',
        '#markup' => $markup,
      );      
    }

    return $element;
  }
  
  /**
   * Create select field.
   * 
   * @param $items
   *   Associative array, where each item will be rendered as an option
   *   with key as the value and value as text.
   * @param $disable
   *   Array of items to match an item if it should be disabled.
   */
  public function create_select($items, $disable) {
    $option = array();
    $cache_exp = [];
    foreach($items as $loc_exp) {
      list($trait_id, $project_id, $project_name, $location) = explode('#', $loc_exp);
      $select_value = $loc_exp['trait_id'] . '#' . $loc_exp['project_id'] . '#' . $loc_exp['location'] . '#' . $loc_exp['germplasm'];
      $cache_exp[] = $loc_exp['project_id'];

      $disabled = (in_array($loc_exp['project_id'], $disable)) ? '' : 'disabled';
      $option[] = '<option value ="' . $select_value . '" ' . $disabled . '>' . strtoupper($loc_exp['location']) . '/' . $loc_exp['project_name'] . '</option>';
    }
  
    $select = '<select class="form-select" id="rawphenotypes-germplasm-field-trait-select-' . $loc_exp['trait_id'] . '">
    <option value="0" selected>Select (' . count($items) . ' Locations, ' . count(array_unique($cache_exp)) . ' Experiments)</option>
    %s
    </select>';

    return sprintf($select, implode('', $option));
  }
}

