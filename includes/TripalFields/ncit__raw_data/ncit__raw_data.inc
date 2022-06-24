<?php
/**
 * @class
 * Purpose: Provide quick browse functionality for entity pages
 *
 * Data: No data.
 * Assumptions:
 */
class ncit__raw_data extends TripalField {
  // --------------------------------------------------------------------------
  //                     EDITABLE STATIC CONSTANTS
  //
  // The following constants SHOULD be set for each descendant class.  They are
  // used by the static functions to provide information to Drupal about
  // the field and it's default widget and formatter.
  // --------------------------------------------------------------------------
  // The default label for this field.
  public static $default_label = 'Germplasm Raw Phenotypes';
  // The default description for this field.
  public static $default_description = 'Show available raw phenotypes of a germplasm';
  // The default widget for this field.
  public static $default_widget = 'ncit__raw_data_widget';
  // The default formatter for this field.
  public static $default_formatter = 'ncit__raw_data_formatter';
  // The module that manages this field.
  public static $module = 'rawphenotypes';

  // A list of global settings. These can be accessed within the
  // globalSettingsForm.  When the globalSettingsForm is submitted then
  // Drupal will automatically change these settings for all fields.
  // Once instances exist for a field type then these settings cannot be
  // changed.
  public static $default_settings = array(
    'storage' => 'tripal_no_storage',
    // It is expected that all fields set a 'value' in the load() function.
    // In many cases, the value may be an associative array of key/value pairs.
    // In order for Tripal to provide context for all data, the keys should
    // be a controlled vocabulary term (e.g. rdfs:type). Keys in the load()
    // function that are supported by the query() function should be
    // listed here.
    'browseable_keys' => array(),
  );

  // Provide a list of instance specific settings. These can be access within
  // the instanceSettingsForm.  When the instanceSettingsForm is submitted
  // then Drupal with automatically change these settings for the instance.
  // It is recommended to put settings at the instance level whenever possible.
  // If you override this variable in a child class be sure to replicate the
  // term_name, term_vocab, term_accession and term_fixed keys as these are
  // required for all TripalFields.
  public static $default_instance_settings = array(
    // The short name for the vocabulary (e.g. schema, SO, GO, PATO, etc.).
    'term_vocabulary' => 'NCIT',
    // The name of the term.
    'term_name' => 'Raw Data',
    // The unique ID (i.e. accession) of the term.
    'term_accession' => 'C142663',
    // Set to TRUE if the site admin is not allowed to change the term
    // type, otherwise the admin can change the term mapped to a field.
    'term_fixed' => FALSE,
    // Indicates if this field should be automatically attached to display
    // or web services or if this field should be loaded separately. This
    // is convenient for speed.  Fields that are slow should for loading
    // should have auto_attach set to FALSE so tha their values can be
    // attached asynchronously.
    'auto_attach' => FALSE,
    // The table where the options for this specific field are stored.
    // This can be one of trpfancy_browse_options or trpfancy_browse_options_per_entity
    // based on admin configuration. Default: trpfancy_browse_options.
    'option_storage' => '',
    // A list of browser types this field intends to provide.
    'browser_types' => '',
  );

  // A boolean specifying that users should not be allowed to create
  // fields and instances of this field type through the UI. Such
  // fields can only be created programmatically with field_create_field()
  // and field_create_instance().
  public static $no_ui = FALSE;
  // A boolean specifying that the field will not contain any data. This
  // should exclude the field from web services or downloads.  An example
  // could be a quick browse field that appears on the page that redirects
  // the user but otherwise provides no data.
  public static $no_data = TRUE;

 /**
   * Loads the field values from the underlying data store.
   *
   * @param $entity
   *
   * @return
   *   An array of the following format:
   *     $entity->{$field_name}['und'][0]['value'] = $value;
   *   where:
   *     - $entity is the entity object to which this field is attached.
   *     - $field_name is the name of this field
   *     - 'und' is the language code (in this case 'und' == undefined)
   *     - 0 is the cardinality.  Increment by 1 when more than one item is
   *       available.
   *     - 'value' is the key indicating the value of this field. It should
   *       always be set.  The value of the 'value' key will be the contents
   *       used for web services and for downloadable content.  The value
   *       should be of the follow format types: 1) A single value (text,
   *       numeric, etc.) 2) An array of key value pair. 3) If multiple entries
   *       then cardinality should incremented and format types 1 and 2 should
   *       be used for each item.
   *   The array may contain as many other keys at the same level as 'value'
   *   but those keys are for internal field use and are not considered the
   *   value of the field.
   */
  public function load($entity) {
    global $user;
    
    // User permissions.
    $rawpheno_permission = array('access rawpheno', 'download rawpheno');
    $count_permission = 0;
    foreach($rawpheno_permission as $permission) {
      if (user_access($permission, $user)) {
        $count_permission++;
      }
    }
    
    // If user has no permission to begin with, skip all and report
    // raw phenotypes not available.
    $field_name = $this->instance['field_name'];
    $entity->{$field_name}['und'][0]['value'] = array();

    if (user_is_logged_in() && $count_permission == 2) {
      // # TRAITS/EXPERIMENT/LOCATION:
      $traits = array();
      // This query is identical to the rawphenotypes download page.
      // Get all experiments (by plant id) where germplasm was used.
      $all_experiment_locations = chado_query("
        SELECT p2.project_id, p2.name, value AS location
        FROM pheno_plantprop 
          INNER JOIN pheno_plant_project AS p1 USING (plant_id) 
          INNER JOIN {project} AS p2 ON p1.project_id = p2.project_id
        WHERE
          type_id = (SELECT cvterm_id FROM {cvterm} cvt LEFT JOIN {cv} cv ON cv.cv_id = cvt.cv_id
            WHERE cvt.name = 'Location' AND cv.name = 'phenotype_plant_property_types') AND
          plant_id IN (SELECT plant_id FROM pheno_plant WHERE stock_id = :germplasm GROUP BY plant_id)
        GROUP BY p2.project_id, p2.name, value
      ", array(':germplasm' => $entity->chado_record->stock_id));
      $experiment_locations = $all_experiment_locations->fetchAll();
      
      // Only when germplasm returned rawphenotypic data.
      if (count($experiment_locations) > 0) {
        // User appointed experiments.
        // See includes/rawpheno.function.measurements.inc file for function definition
        // Given the user id this function returns an array of chado projects keyed by project_id 
        // that the user has permission to see.
        $user_experiment = rawpheno_function_user_project($user->uid); 
        $user_experiment = array_keys($user_experiment); 

        // All traits in experiment and location.
        $sql_cvterm = "
          SELECT c_j.cvterm_json->>'id', c_j.cvterm_json->>'name' FROM (
            SELECT JSON_BUILD_OBJECT('id', cvterm_id, 'name', name) AS cvterm_json FROM {cvterm} WHERE cvterm_id = ANY ((
              SELECT STRING_TO_ARRAY(list_id.all_traits, ',') FROM (
                SELECT string_agg(DISTINCT all_traits, ',') AS all_traits
                FROM {rawpheno_rawdata_mview}
                WHERE
                  location IN(:location)
                  AND plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id = :project_id)
              ) AS list_id
            )::int[])
          ) AS c_j
          WHERE c_j.cvterm_json->>'name' NOT IN ('Rep', 'Entry', 'Location', 'Name', 'Plot', 'Planting Date (date)', '# of Seeds Planted (count)')
          ORDER BY c_j.cvterm_json->>'name' ASC
        ";
        
        $trait_experiment_location = array();
        $cache_exp = array();
        $cache_loc = array();

        foreach($experiment_locations as $item) {
          $cache_exp[] = $item->project_id;
          $cache_loc[] = $item->location;

          $trait_set = chado_query($sql_cvterm, array(':location' => $item->location, ':project_id' => $item->project_id))
            ->fetchAllKeyed(0, 1);
          
          if ($trait_set) {
            foreach($trait_set as $trait_id => $trait_name) {     
              $allow = (in_array($item->project_id, $user_experiment)) ? 1 : 0;

              // Save basic information about the trait (name + id key) and all experiment + location it was measured.
              $entity->{$field_name}['und'][0]['value']['hydra:member'][ $trait_name . '_' . $trait_id ][] = array(                              
                'phenotype_customfield_terms:id'  => $item->project_id,    // Project id number.
                'phenotype_customfield_terms:name'  => $item->name,        // Project name. 
                'phenotype_customfield_terms:location' => $item->location, // Location in a project trait was measured.
                'phenotype_customfield_terms:user_experiment' => $allow    // Does user have permission? 
              );
            }
          }
        }
        
        // Save a complete summary count as a quick raw phenotypic data summary related to the germplasm. 
        $entity->{$field_name}['und'][0]['value']['phenotype_customfield_terms:summary'] = array(        
          'phenotype_customfield_terms:experiment' => count(array_unique($cache_exp)), // Summary count of experiments.
          'phenotype_customfield_terms:location' => count(array_unique($cache_loc)),   // Summary count of locations.
          'phenotype_customfield_terms:trait' => count($entity->{$field_name}['und'][0]['value']['hydra:member']), // Summary count of traits.
        );
      }
    }
  }

  /**
   * Provides a form for the 'Field Settings' of an instance of this field.
   *
   * This function corresponds to the hook_field_instance_settings_form()
   * function of the Drupal Field API.
   *
   * Validation of the instance settings form is not supported by Drupal, but
   * the TripalField class does provide a mechanism for supporting validation.
   * To allow for validation of your setting form you must call the parent
   * in your child class:
   *
   * @code
   *   $element = parent::instanceSettingsForm();
   * @endcode
   *
   * Please note, the form generated with this function does not easily
   * support AJAX calls in the same way that other Drupal forms do.  If you
   * need to use AJAX you must manually alter the $form in your ajax call.
   * The typical way to handle updating the form via an AJAX call is to make
   * the changes in the form function itself but that doesn't work here.
   */
  public function instanceSettingsForm() {

    // Retrieve the current settings.
    // If this field was just created these will contain the default values.
    $settings = $this->instance['settings'];

    // Allow the parent Tripal Field to set up the form element for us.
    $element = parent::instanceSettingsForm();

    return $element;
  }

  /**
   * @see ChadoField::elementInfo()
   *
   */
  public function elementInfo() {
    $field_term = $this->getFieldTermID();
    return array(
      $field_term => array(
        'operations' => array('eq', 'ne', 'contains', 'starts'),
        'sortable' => TRUE,
        'searchable' => TRUE,
      ),
    );
  }
}