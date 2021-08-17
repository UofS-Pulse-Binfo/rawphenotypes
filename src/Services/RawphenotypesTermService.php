<?php
/**
 * @file
 * Contains class definition of RawphenotypesTermsService.
 * This class will provide management of CV and Terms used by this module
 * by using core Tripal vocabulary and term management service.
 * 
 * All vocabulary terms used by this module fall under the following
 * controlled vocabulary terms.
 * 
 * - rawphenotypes_terms - each cv following will be a term under this cv.
 * - phenotype_plant_property_types - plant properties terms (name, entry).
 * - phenotype_measurement_units - measurement units (cm, g).
 * - phenotype_measurement_types - traits terms (plant height(cm)).
 * - phenotype_r_compatible_version - R compatible version of trait.
 * - phenotype_collection_method - collection method (descriptions).
 * 
 * NOTE: terms in rawphenotypes_terms cv will be used to create
 * relationships in cvtermprop and cvterm_relationship as value
 * to the field type_id.
 * 
 * @see tripal\Services\TripalVocabManager
 * @see tripal\Services\TripalTermManager
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesTermService {
  // Hold all controlled vocabulary terms/names listed in the comment.
  private $vocabularies;
  // R-transformation rules set by configuration.
  private $rtransform_rules;
  // Table to store term properties (method information, r-version);
  private $table_property = 'cvtermprop';
  // Table to store term relationships (term-unit).
  private $table_relationship = 'cvterm_relationship';
  // Term/trait replicates (r1, r2...)
  private $replicates;

  public function __construct() {
    $default_service = \Drupal::service('rawphenotypes.default_service');
    $this->vocabularies = $default_service::getDefaultValue('vocabularies');

    $settings = \Drupal::service('config.factory')
      ->getEditable('rawphenotypes.settings');

    $this->rtransform_rules = [
      'word_rules' => $settings->get('rawpheno_rtransform_words'),
      'char_rules' => $settings->get('rawpheno_rtransform_characters'),
      'replace_rules' => $settings->get('rawpheno_rtransform_replace'),
    ];

    $this->replicates = $default_service::getTraitReps();
  }

  /**
   * Create or add controlled vocabulary terms.
   * 
   * @param $details
   *   Associative array where key corresponds to fields in cv or cvterm.
   *   Fields/Key:
   *   - name
   *   - definition
   *   - cv_name/cv_id
   * 
   * @return object
   *   Inserted term row object.
   * 
   * @TODO: Update to comply with Tripal 4 terms and vocab services only
   * when term service used for non-content type become available.
   */
  public function addTerm($details) {
    $term = (function_exists('chado_insert_cvterm'))
      ? chado_insert_cvterm($details)
      : chado_insert_cvterm($details);
  
    return $term ?? null;
  }

  /**
   * Update values name and definition values of a term.
   * 
   * @param $term_id
   *   Integer, term id/cvterm id number to be updated.
   * @param $new_name
   *   New term name to replace existing term name.
   * @param $new_definition
   *   New term definition to replace existing term definition.
   * 
   * @TODO: Update to comply with Tripal 4 terms and vocab services only
   * when term service used for non-content type become available.
   */
  public function updateTerm($term_id, $new_name, $new_definition): void {
    $table = 'cvterm';
    $match = ['cvterm_id' => $term_id];

    chado_update_record($table, 
      $match,
      [
        'name' => $new_name,
        'definition' => $new_definition
      ]
    );
  }

  /**
   * Get controlled vocabulary term or cv terms using name field.
   * 
   * @param $term_name
   *   String, term or vocabulary term name value.
   * @param $source
   *   String, source table to search the term name.
   *   Default to chado.cvterm.
   * 
   * @return object
   *   Term row matching the name (cv.name or cvterm.name) as an object.
   */
  public function getTermByName($term_name, $source = 'cvterm') {
    if ($source == 'cv') {
      // Term vocabulary source - find in chado.cv.
      $table = 'chado.cv';
      $primary_key = 'cv_id';
    }
    else {
      // Term, cvterm source - find in chado.cvterm.
      $table = 'chado.cvterm';
      $primary_key = 'cvterm_id';
    }

    $term_name = trim($term_name);

    // This is a case-sensitive search so terms Null and null are considered
    // two difference terms.
    $sql = 'SELECT %s, name, definition FROM %s WHERE name = :term_name';
    $sql = sprintf($sql, $primary_key, $table);
    $args = [':term_name' => $term_name];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    return $query->fetchObject() ?? null;
  }

  /**
   * Get term.
   * 
   * @param $details
   *   Associative array, where key corresponds to field in chado.cvterm or chado.cv. 
   * @param $source
   *   Source table cv or cvterm. Default to cvterm.
   * 
   * @return object
   *   CV term row object.
   */
  public function getTerm($details, $source = 'cvterm') {
    if ($source == 'cv') {
      return (function_exists('chado_get_cv')) 
        ? chado_get_cv($details)
        : tripal_get_cv($details);
    }
    else {
      return (function_exists('chado_get_cvterm')) 
        ? chado_get_cvterm($details)
        : tripal_get_cvterm($details);
    }
  }

  /**
   * Base on R transformation rules, convert a term into 
   * R-compatible version. This version can be reference using
   * the relationship between a term and r-version in cvtermprop table.
   * 
   * Transformation rules can be set in the control panel of this module.
   *
   * @param $column_header
   *   A string containing the column header to transform.
   * 
   * @return
   *   A string containing the R compatible column header.
   */
  public function makeTermRCompatible($column_header) {
    // Get R transformation rules set in the admin control panel.
    $word_rules = $this->rtransform_rules['word_rules'];
    $char_rules = $this->rtransform_rules['char_rules'];
    $replace_rules = $this->rtransform_rules['replace_rules'];

    $arr_match = [];
    $arr_replace = [];
    $r = explode(',', $replace_rules);

    // Convert the rule to key and value pair. The key is the matching character/word and
    // the values is the replacement value when key is found in a string.
    foreach($r as $g) {
      list($match, $replace) = explode('=', $g);
      $arr_match[] = trim($match);
      $arr_replace[] = trim($replace);
    }

    // Convert special characters transformation rules in string to array.
    $char_rules = explode(',', $char_rules);

    // Convert words transformation rules in string to array.
    $word_rules = explode(',', $word_rules);

    // Remove leading and trailing spaces from the selected trait.
    // Convert string to lowercase.
    $selected_trait = trim(strtolower($column_header));
    // 1. Break the column header in string to individual words,
    //    and remove all words that matches an entry in the words transfomation rules.
    $w = explode(' ', $selected_trait);
    foreach($w as $c) {
      $c = trim($c);

      // Skip the words in the traits that are present in the
      // words transformation rules.
      if (!in_array($c, $word_rules)) {
        // Do match and replace, as well as, removal of special characters
        // only when the current word is not in the words transformation rules.
        // 2. Match and replace based on match and replace rule.
        $c = str_replace($arr_match, $arr_replace, $c);
        // 3. Remove all special characters listed in remove chars rule.
        $c = str_replace($char_rules, '', $c);

        // All transformation rules applied, make sure that
        // the result is not a blank space.
        if (!empty($c)) {
          $rfriendly[] = trim($c);
        }
      }
    }

    // Final transformation is replacing all spaces to dots/period (.)
    return ucfirst(implode('.', $rfriendly));
  }

  /**
   * Parse a line of text which is a term/column header to find and
   * extract the unit part present in the string.
   *
   * @param $header
   *   A string containing the header. Usually following
   *   the Trait name (unit) format.
   *
   * @return
   *   A string containing the unit of the header.
   */
  public function getTermUnit($header) {
    // Find the unit part in the string (unit).
    // Not found return text.
    preg_match("/.*\(([^)]*)\)/", $header, $match);
    $u = (isset($match[1])) ? $match[1] : 'text';

    // Unit can contain replicates, fetch all replicates
    // used and remove it from the unit.
    $chars = $this->replicates;
    array_push($chars, ';', ': 1-5');
    $unit = str_ireplace($chars, '', $u);

    return trim(strtolower($unit));
  }

  /**
   * Set data collection method of a trait.
   * Example: trait method and trait R version.
   * 
   * Key and value expected.
   * - cvterm_id: term id number.
   * - type_id: describes the property created, it is the cvterm_id value
   *   corresponding to phenotype_collection_method and phenotype_r_compatible_version.
   * - value: method description or the r transformed value.
   * - rank: default and set to 0.
   * 
   * @param $details
   *   Associative array, where key corresponds to field in chado.cvtermprop table.
   */
  public function saveTermProperty($details): void {
    if ($details) {
      chado_insert_record($this->table_property, $details);
    }
  }

  /**
   * Update a term property whether in chado specific database
   * and module specific table.
   * 
   * @param $term_id 
   *   Integer, term id/cvterm id number to be updated.
   * @param $type_id
   *   Integer, type id number corresponding to which property type to be updated.
   * @param $value
   *   New value to replace.
   * @param $project
   *   Integer, project id number used for property that requires project specific constraints.
   *   Default to null.
   * @param $chadoprop
   *   Boolean, True if property is from a chado table and False if from custom table.
   *   Default to TRUE - updates to chado table.
   */
  public function updateTermProperty($term_id, $type_id, $value, $project = null, $chadoprop = TRUE) {    
    if ($chadoprop) {
      // Update term method or r-version value from a 
      // chado table.
      $table = $this->table_property;
      $match = ['cvterm_id' => $term_id, 'type_id' => $type_id];

      chado_update_record($table, 
        $match,
        ['value' => $value]
      );
    }
    else {
      // Update term type whether essential, optional or custom type.
      // Source table is custom table.
      $table = 'pheno_project_cvterm';
      \Drupal::database()
        ->update($table)
        ->fields(['type' => $value])
        ->condition('cvterm_id', $term_id, '=')
        ->condition('project_id', $project, '=')
        ->execute();
    }
  }

  /**
   * Get term R version or Method property.
   * 
   * @param $term_id
   *   Integer, term id/cvterm id number to be updated.
   * @param $property
   *   String, method or rversion property of a term.
   *   [method or rversion], default to method property.
   * 
   * @return string
   *   Method or Rversion information of a term.
   */
  public function getTermProperty($term_id, $property = 'method') {    
    $properties = [];
    $properties['method'] = $this->vocabularies['cv_desc'];
    $properties['rversion'] = $this->vocabularies['cv_rver'];

    $property = $properties[ $property ];
    // Get the cvterm equivalent of a cv used in tagging a property (type_id).
    $type = self::getTerm(['name' => $property], ['cv_id' => ['name' => $this->vocabularies['cv_phenotypes']]]);

    $sql = "SELECT value FROM chado.cvtermprop WHERE cvterm_id = :term_id AND type_id = :cv_id";
    $args = [':term_id' => $term_id, ':cv_id' => $type->cvterm_id];
    
    $property_value = \Drupal::database()
      ->query($sql, $args)
      ->fetchField();
    
    return $property_value ?? null;
  }

  /**
   * Create trait relationship ie. trait-unit relationship.
   * 
   * Key and value expected.
   * - subject_id: the term id - the unit cvterm id.
   * - type_id: cvterm id for term phenotype_measurement_units.
   * - object_id: term id - the trait/header cvterm id.
   * 
   * @param $details
   *   Associative array, where key corresponds to field in chado.cvterm_relationship table.
   */
  public function saveTermRelationship($details): void {
    if ($details) {
      chado_insert_record($this->table_relationship, $details);
    }
  }

  /**
   * Update term relationship.
   * 
   * @param $term_id
   *   Integer, term id/cvterm id number to be updated.
   * @param $value
   *   New value to replace the existing relationship.
   *   Value will be the subject in the relationship.
   */
  public function updateTermRelationship($term_id, $value): void {
    if ($value) {
      $match = ['object_id' => $term_id];
      chado_update_record($this->table_relationship, 
        $match,
        [
          'subject_id' => $value
        ]
      );
    }
  }
  
  /**
   * Assign term to a project.
   * 
   * @param $term_id
   *   Integer, term/trait id.
   * @param $type
   *   String, type (essential, optional, subset or plant property).
   * @param $project_id
   *   Integer, project id to where a term is to be assigned.
   */
  public function saveTermToProject($term_id, $type, $project_id): void {
    $table = 'pheno_project_cvterm';
    $type  = trim($type);
    $project_id = trim($project_id);

    \Drupal::database()
      ->insert($table)
      ->fields([        
        'cvterm_id' => $term_id,
        'type' => $type,
        'project_id' => $project_id,
      ])
      ->execute();
  }

  /**
   * For types using scale as unit, create each scale item.
   * 
   * @param $term_id
   *   Term id measuring scale values.
   * @param $scales
   *   Scale items or range of values.
   */
  public function setTermScaleValues($term_id, $scales) {
    $table = 'pheno_scale_member';
    
    $query = \Drupal::database()
      ->insert($table)
      ->fields(['scale_id', 'code', 'value']);
      
    foreach($scales as $code => $value) {
      $query->fields([
        'scale_id' => $term_id,
        'code' => $code,
        'value' => $value
      ]);
    }

    $query->execute();    
  }

  /**
   * Get all plant property type column headers.
   */
  public function getPlantPropertyTerm() {
    $sql = "
      SELECT cvterm_id
      FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING(cv_id)
      WHERE t1.name = :plantprop_type
      ORDER BY cvterm_id ASC
    ";
    $args = [':plantprop_type' => $this->vocabularies['cv_prop']];

    $query = \Drupal::database()
      ->query($sql, $args);

    $query->allowRowCount = TRUE;

    return ($query->rowCount()) ? $query : [];  
  }

  /**
   * Get all terms by type.
   * 
   * @param $type
   *   Type of term that corresponds to the cv_id of the cvterm row.
   *   One of the following:
   *     phenotype_plant_property_types
   *     phenotype_measurement_units
   *     phenotype_measurement_types
   *     phenotype_r_compatible_version
   *     phenotype_collection_method
   * 
   * @param return 
   *   Array, rows matching the type (cv_id) in cvterm table.
   */
  public function getTermsByType($type) {
    $sql = "
      SELECT t2.name, t2.definition
      FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING (cv_id)
      WHERE t1.name = :type
      ORDER BY t2.name ASC
    ";
    $args = [':type' => $type];

    $query = \Drupal::database()
      ->query($sql, $args);

    $query->allowRowCount = TRUE;

    return ($query->rowCount()) ? $query->fetchAllKeyed() : [];
  }

  /**
   * Get terms that are not listed as among the traits in a project.
   * 
   * @param $project_id
   *   Integer, Project id number terms should not be registered to.
   * 
   * @return array
   *   Term rows in chado.cvterm.
   */
  public function getTermsNotInProject($project_id) {
    $sql = "
      SELECT t2.cvterm_id, t2.name
      FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING(cv_id)
      WHERE t1.name = 'phenotype_measurement_types'
        AND t2.cvterm_id NOT IN (SELECT cvterm_id FROM pheno_project_cvterm WHERE project_id = :project_id)
      ORDER BY t2.cvterm_id ASC
    ";
    $args = [':project_id' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount()) ? $query : [];
  }

  /**
   * Get term properties.
   * 
   * @param $term_id
   *   Integer, term or cvterm id number.
   * @param $dataset
   *   An string indicating whether to include data count.
   *
   * @return
   *   An array containing all properties (project, name, data, etc.) of a column header.
   */
  public function getTermProperties($term_id, $dataset = null) {
    // Array to hold properties.
    $arr_properties = [];

    // Get project information and header type.
    $sql = "
      SELECT t1.project_id, t1.name, t2.cvterm_id, t2.type
      FROM chado.project AS t1 INNER JOIN pheno_project_cvterm AS t2 USING(project_id)
      WHERE t2.project_cvterm_id = :record_id LIMIT 1
    ";
    $args = [':record_id' => $term_id];
    
    $h = \Drupal::database()
      ->query($sql, $args)
      ->fetchObject();

    $arr_properties['in_project_id'] = $h->project_id;
    $arr_properties['in_project_name'] = $h->name;
    $arr_properties['cvterm_id'] = $h->cvterm_id;
    $arr_properties['type'] = $h->type;
      
    $h = self::getTerm(['cvterm_id' => $arr_properties['cvterm_id']]);
    $arr_properties['name'] = $h->name;
    $arr_properties['definition'] = empty($h->definition) ? '' : $h->definition;

    // cvterm R Version and Collection Method.
    $arr_properties['method'] = '';
    
    $sql = "
      SELECT value, type_id FROM chado.cvtermprop
      WHERE cvterm_id = :record_id
    ";
    
    $args = [':record_id' => $arr_properties['cvterm_id']];
    $h = \Drupal::database()
      ->query($sql, $args);
    
    $rversion_prop = self::getTermByName($this->vocabularies['cv_rver']);
    $method_prop = self::getTermByName($this->vocabularies['cv_desc']);

    // From the query which will return both properties (method and r version),
    // Identify which is method from r version and tag accordingly using 
    // corresponding property id numbers.
    foreach($h as $c) {
      $val = isset($c->value) ? $c->value : '';

      if ($c->type_id == $rversion_prop->cvterm_id) {
        $arr_properties['r_version'] = $val;
      }
      elseif ($c->type_id == $method_prop->cvterm_id) {
        $arr_properties['method'] = $val;
      }
    }

    // Request full dataset including basic stats about the header.
    if ($dataset == 'full') {
      // Count data associated to column header.
      $sql = "
        SELECT COUNT(type_id) AS data_count FROM pheno_measurements
        WHERE type_id = :cvterm_id 
          AND plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id = :project_id)
      ";
      $args = [':project_id' => $arr_properties['in_project_id'], ':cvterm_id' => $arr_properties['cvterm_id']];

      $h = \Drupal::database() 
        ->query($sql, $args)
        ->fetchObject();

      $arr_properties['count_data'] = $h->data_count;

      // Count the projects this same column header is being used.
      $sql = "
        SELECT COUNT(project_id) AS project_count
        FROM pheno_project_cvterm 
        WHERE project_id <> :project_id AND cvterm_id = :cvterm_id
      ";

      $h = \Drupal::database() 
        ->query($sql, $args)
        ->fetchObject();

      $arr_properties['count_project'] = $h->project_count;
    }

    return $arr_properties;
  }

 /**
  * Construct a trait given a name, trait rep and unit.
  *
  * @param $trait
  *   An array containing name, trait and unit.
  *
  * @return
  *   A string containing containing the column header in name (trait rep; unit) format.
  */
  public function constructTerm($trait) {
    $unit = '';

    if (!empty($trait['unit']) OR !empty($trait['rep'])) {
      $u = (empty($trait['rep'])) ? '' : $trait['rep'] . ' ';
      if (empty($trait['unit'])) {
        $u = trim($u);
      }

      $unit = '(' . $u . $trait['unit'] . ')';
    }

    $name = ucfirst($trait['name'] . ' ' . rtrim($unit));

    return trim($name);
  }

  /**
   * Removes a term from project.
   * Deassociate the term from a project and not physical deletion of the 
   * trait record.
   * 
   * @param $term_id
   *   Integer, term id number that corresponds to cvterm_id (trait/header id).
   * @param $project_id
   *   Integer, project id number the term is part of.
   */
  public function removeTermFromProject($term_id, $project_id): void {
    $table = 'pheno_project_cvterm';
    \Drupal::database()
      ->delete($table)
      ->condition('cvterm_id', $term_id)
      ->condition('project_id', $project_id)
      ->execute();
  }

  /**
   * Function that list default/initial units available to this module.
   *
   * @param $set
   *   A string indicating the type of set to return.
   *   def - unit and definition.
   *   type - unit and data type.
   */
  public function getUnitDataType($set) {
    // Type is required when programmatically generating data collection spreadsheet file
    // in instructions page.
    return [
      'date'  => ($set == 'def') ? 'Date'  : 'date',
      'count' => ($set == 'def') ? 'Count' : 'integer',
      'days'  => ($set == 'def') ? 'Days'  : 'integer',
      'cm'    => ($set == 'def') ? 'Centimeters'  : 'integer',
      'scale' => ($set == 'def') ? 'Scale: 1-5'   : 'integer',
      'g'     => ($set == 'def') ? 'Grams (g)'    : 'integer',
      'text'  => ($set == 'def') ? 'Alphanumeric' : 'string',
      'y/n/?' => ($set == 'def') ? 'Yes, No or ? - Not sure' : 'string'
    ];
  }
}