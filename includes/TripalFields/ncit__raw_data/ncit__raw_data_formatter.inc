<?php
/**
 * @class
 * Purpose: Provide a summary of raw phenotypic data to a germplasm.
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
    // Only when germplasm has raw phenotypic data under experiment/s that user
    // has permission to access or export data. This value is an empty array when
    // user has neither permission nor active experiments.
    if ($items[0]['value']) {
      // All trait and experiment+location values are accessible through this var.
      $germplasm_raw_phenotypes = $items[0]['value']['hydra:member'];
      // Overall summary count of raw phenotypic data by experiment, location and trait.
      $summary_values = $items[0]['value']['phenotype_customfield_terms:summary'];
      // Id (stock id) of the current germplasm.
      $germplasm_id = $entity->chado_record->stock_id;
      // Name (stock name) of the current germplasm.
      $germplasm_name = $entity->chado_record->name;

      // Reference directory path.
      $base_path   = $GLOBALS['base_url'] . '/'; 
      $module_path = drupal_get_path('module', 'rawpheno') . '/';
      $theme_path  = $base_path . $module_path . '/includes/TripalFields/ncit__raw_data/theme/';
      
      // Append image as bullet points, header icon and export button or link.
      $img = '<img id="rawphenotypes-germplasm-field-filterby-%s-img" src="%s" border="0" align="absmiddle" title="%s" />';
      

      // CONSTRUCT SUMMARY TABLE:
      // Each row will contain the trait name (trait header), location and experiment combination (LOCATION/Experiment header)
      // as a select box and download link (download icon header).

      // # TABLE HEADER:
      $table_header = array(
        sprintf($img, '', $theme_path . 'icon-download-all.jpg', 'Download all for this trait'), 
        'Trait', 
        'Filter by:',
        sprintf($img, '', $theme_path . 'icon-download.jpg', 'Download')
      );
      
      // 2 select fields are required:
      // A. Select field to filter by Location + Experiment.
      // B. Select field to filter by Experiment (All location included).
      // Depending on which Filter by option user selects, load corresponding select.
            
      // # TABLE ROWS:
      $table_row = array();       
      foreach($germplasm_raw_phenotypes as $trait => $exp_loc) {
        $tmp = explode('_', $trait);
        $trait = array('id' => $tmp[1], 'name' => $tmp[0]);

        $table_row[] = array(
          sprintf($img, $trait['id'] . '-all', $theme_path . 'icon-export.png', 'Download all for this trait'),
          ucfirst($trait['name']), 
          $this->create_select($trait['id']),
          sprintf($img, $trait['id'], $theme_path . 'icon-export.png', 'Download')
        ); 
      }
      
      // # THEME TABLE:
      $summary_table = theme('table', array(
        'header' => $table_header, 
        'rows' => $table_row, 
        'sticky' => FALSE, 
        'attributes' => array('id' => 'rawphenotypes-germplasm-field-table'))
      );
      

      // Make field elements generated by formatter avaiable to the template as template vars.
      // @see template file for this field in rawphenotypes/theme directory.
      $markup = theme('rawpheno_germplasm_field', array(
        'element_id' => 'rawphenotypes-germplasm-raw-phenotypes-field',
        'summary_table' => array(
          'table' => $summary_table,
          'headers' => array(
            'germplasm'   => $germplasm_name,
            'experiments' => $summary_values['phenotype_customfield_terms:experiment'],   
            'locations'   => $summary_values['phenotype_customfield_terms:location'],
            'traits'      => $summary_values['phenotype_customfield_terms:trait'], 
          )
        )
      ));

      // Rawphenotypes download link:
      drupal_add_js(array('rawpheno' => array('exportLink' => $base_path . '/phenotypes/raw/download')), array('type' => 'setting'));
      // Autocomplete UI.
      drupal_add_library('system', 'ui.autocomplete');
      // All datapoints available to JS to populate select field.
      drupal_add_js(array('rawpheno' => array('germRawdata' => $germplasm_raw_phenotypes, 'germ' => $germplasm_id)), array('type' => 'setting'));


      // Construct field render array.
      $element[0] = array(
        '#type' => 'markup',
        '#markup' => $markup,
        '#attached' => array(
          'css' => array($module_path . 'theme/css/rawpheno.germplasmfield.style.css'),
          'js'  => array($module_path . 'theme/js/rawpheno.germplasmfield.script.js')
        )
      );      
    }

    return $element;
  }
  
  /**
   * Create select field.
   * 
   * @param $trait_id
   *   Trait id number (cvterm id number) used as value for id attribute.
   */
  public function create_select($trait_id) {
    $attributes = array(
      'id' => 'rawphenotypes-germplasm-field-filterby-' .  $trait_id,
      'class' => array('rawphenotypes-germplasm-field-filterby')
    );

    $options = array(
      0 => '---'
    );

    return theme('select', array('element' => array('#attributes' => $attributes, '#options' => $options)));
  }
}