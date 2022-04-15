<?php
/**
 * @file
 * Contains all field specific code outside the classes.
 */

/**
 * Implements hook_bundle_fields_info().
 *
 * This hook tells Drupal/Tripal about your new field type. Make sure you've created the
 * field (handles basic storage of your data), widget (provides user UI to set data),
 * and formatter (describes display of data on Entity Page) classes. These should be
 * located in the following directory: [your module]/includes/TripalFields/[classname].inc
 * with one file per class. Your field name should be [cv name]__[cvterm name] and the
 * classes should be named [field name], [field_name]_widget, [field name]_formatter
 * for the field, widget and formatter respectively. MAKE SURE YOU'VE CLEARED THE CACHE
 * SINCE ADDING THESE FILES so Tripal magic can find them or the following will fail.
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene) this hook is
 *   being called for. However, since this hook creates field types (by definition not
 *   tied to a specific Tripal Content Type (bundle)) and since a field type will only be
 *   created if it doesn't already exist, this parameter doesn't actually matter.
 *   NOTE: If you do need to determine the bundle in this hook, we suggest inspecting
 *   the data_table since the label can be changed by site administrators.
 *
 * @return
 *   An array of field definitions. Each field in this array will be created if it
 *   doesn't already exist. To trigger create of fields when developing call
 *   tripal_refresh_bundle_fields() for the specific bundle.
 */
function rawpheno_bundle_fields_info($entity_type, $bundle) {
  $fields = array();

  // IN GERMPLASM PAGE ONLY:
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {
    // Insert auxiliary terms used by Raw Data field.
    // Create cv to hold terms.
    $cv = 'phenotype_rawdatafield_terms';
    chado_insert_cv($cv, 'vocabulary term to hold terms used by Raw Data field');
    
    // Insert terms to cv above.
    $terms = array('experiment', 'id', 'location', 'name', 'permission', 'summary', 'trait');
    foreach($terms as $term) {
      tripal_insert_cvterm(array(
        'id' => 'rawpheno_tripal:' . $term,
        'name' => $term,
        'definition' => $term,
        'cv_name' => $cv
      ));
    }

    // Raw Data term.
    tripal_insert_cvterm(array(
      'id' => 'NCIT:C142663',
      'name' => 'Raw Data',
      'cv_name' => 'NCIT',
      'definition' => 'The original information, collected from the primary source. Used in Germplasm Raw Phenotypes Field.',
    ));

    $field_name = 'ncit__raw_data';
    $field_type = 'ncit__raw_data';
    $fields[$field_name] = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => 1,
      'locked' => FALSE,
      'storage' => array(
	      'type' => 'field_chado_storage',
      ),
    );
  }

  return $fields;
}

/**
 * Implements hook_bundle_instances_info().
 *
 * This hook tells Drupal/Tripal to create a field instance of a given field type on a
 * specific Tripal Content type (otherwise known as the bundle). Make sure to implement
 * hook_bundle_create_fields() to create your field type before trying to create an
 * instance of that field.
 *
 * @param $entity_type
 *   This should be 'TripalEntity' for all Tripal Content.
 * @param $bundle
 *   This object describes the Type of Tripal Entity (e.g. Organism or Gene) the field
 *   instances are being created for. Thus this hook is called once per Tripal Content Type on your
 *   site. The name of the bundle is the machine name of the type (e.g. bio_data_1) and
 *   the label of the bundle (e.g. Organism) is what you see in the interface. Since the
 *   label can be changed by site admin, we suggest checking the data_table to determine
 *   if this is the entity you want to add field instances to.
 * @return
 *   An array of field instance definitions. This is where you can define the defaults
 *   for any settings you use in your field. Each entry in this array will be used to
 *   create an instance of an already existing field.
 */
function rawpheno_bundle_instances_info($entity_type, $bundle) {
  $instances = array();

  // IN GERMPLASM PAGE ONLY:
  if (isset($bundle->data_table) AND ($bundle->data_table == 'stock')) {
    // Number of Values Recorded.
    $field_name = 'ncit__raw_data';
    $field_type = 'ncit__raw_data';
    $instances[$field_name] =  array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => $bundle->name,
      'label' => 'Germplasm Raw Phenotypes',
      'description' => 'Field to add interface to raw phenotypes available to a germplasm.',
      'required' => FALSE,
      'settings' => array(
        'term_vocabulary' => 'NCIT',
        'term_name' => 'Raw Data',
        'term_accession' => 'C142663',
        'auto_attach' => FALSE,
        'chado_table' => $bundle->data_table,
        'chado_column' => $bundle->data_table . '_id',
        'base_table' => $bundle->data_table,
      ),
      'widget' => array(
        'type' => 'ncit__raw_data_widget',
        'settings' => array(),
      ),
      'display' => array(
        'default' => array(
          'label' => 'hidden',
          'type' => 'ncit__raw_data_formatter',
          'settings' => array(),
        ),
      ),
    );
  }
  
  return $instances;
}