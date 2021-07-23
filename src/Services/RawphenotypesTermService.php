<?php
/**
 * @file
 * Contains class definition of RawphenotypesTermsService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesTermService {
  /**
   * Add terms cv or cvterms.
   * 
   * @param $details
   *   Associative array where key corresponds to fields in cv or cvterm.
   * @param $table
   *   Table - cv or cvterm to insert. Default to cvterm.
   * 
   * @return object
   *   Inserted term row object.
   * 
   * @TODO: Update to comply with Tripal 4 terms and vocab services.
   */
  public static function addTerm($details, $table = 'cvterm') {
    if ($table == 'cv') {
      list($cv_name, $cv_description) = $details;
      $term = (function_exists('chado_insert_cv')) 
        ? chado_insert_cv($cv_name, $cv_description)
        : tripal_insert_cv($cv_name, $cv_description);
    }
    else {
      $term = (function_exists('chado_insert_cvterm'))
        ? chado_insert_cvterm($details)
        : chado_insert_cvterm($details);
    }    

    return $term ?? null;
  }

  /**
   * Get cvterm or cv by name.
   * 
   * @param $term_name
   *   String, term or vocabulary term name value.
   * @param $source
   *   String, source table to search the term name.
   *   Default to chado.cvterm.
   * 
   * @return 
   */
  public static function getTermByName($term_name, $source = 'cvterm') {
    if ($source == 'cv') {
      // Term vocabulary source - find in chado.cv.
      $table = 'chado.cv';
    }
    else {
      // Term, cvterm source - find in chado.cvterm.
      $table = 'chado.cvterm';
    }

    $term_name = trim($term_name);
    $sql = sprintf('SELECT * FROM %s WHERE name = :term_name', $table);
    $args = [':term_name' => $term_name];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount()) ? $query->fetchObject() : null;
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
  public static function getTerm($details, $source = 'cvterm') {
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
   * Function to transform a column header to R compatible version.
   *
   * @params $column_header
   *   A string containing the column header to transform.
   * @return
   *   A string containing the R compatible column header.
   */
  public static function makeTermRCompatible($column_header) {
    $settings = \Drupal::service('config.factory')
      ->getEditable('rawphenotypes.settings');

    // Get R transformation rules set in the admin control panel.
    $word_rules = $settings->get('rawpheno_rtransform_words');
    $char_rules = $settings->get('rawpheno_rtransform_characters');
    $replace_rules = $settings->get('rawpheno_rtransform_replace');

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
   * Function extract the unit from a column header.
   *
   * @param $header
   *   A string containing the header.
   *
   * @return
   *   A string containing the unit of the header.
   */
  public static function getTermUnit($header) {
    $default_service = \Drupal::service('rawphenotypes.default_service');

    preg_match("/.*\(([^)]*)\)/", $header, $match);
    $u = (isset($match[1])) ? $match[1] : 'text';

    $chars = $default_service::getTraitReps();
    array_push($chars, ';', ': 1-5');

    $unit = str_ireplace($chars, '', $u);

    return trim(strtolower($unit));
  }

  /**
   * Set data collection method of a trait.
   * Example: trait method and trait R version.
   * 
   * @param $details
   *   Associative array, where key corresponds to field in chado.cvtermprop table.
   */
  public static function saveTermProperty($details) {
    $table = 'cvtermprop';
    chado_insert_record($table, $details);
  }

  /**
   * Set trait relationship ie. trait-unit relationship.
   * 
   * @param $details
   *   Associative array, where key corresponds to field in chado.cvterm_relationship table.
   */
  public static function saveTermRelationship($details) {
    $table = 'cvterm_relationship';
    chado_insert_record($table, $details);
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
  public static function saveTermToProject($term_id, $type, $project_id) {
    $table = 'pheno_project_cvterm';
    $term  = trim($term);
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
  public static function setTermScaleValues($term_id, $scales) {
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
  public static function getPlantPropertyTerm() {
    $sql = "
      SELECT cvterm_id
      FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING(cv_id)
      WHERE t1.name = 'phenotype_plant_property_types'
      ORDER BY cvterm_id ASC
    ";

    $query = \Drupal::database()
      ->query($sql);

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
  public static function getTermsByType($type) {
    $sql = "
      SELECT t2.name, t2.name || ' : ' || t2.definition
      FROM chado.cv AS t1 INNER JOIN chado.cvterm AS t2 USING (cv_id)
      WHERE t1.name = :type AND t2.name != :type_name
      ORDER BY t2.name ASC
    ";
    $args = [':type' => $type, ':type_name' => $type];

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
  public static function getTermsNotInProject($project_id) {
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
  public static function getTermProperties($term_id, $dataset = null) {
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
    $h = \Drupal::database()
      ->query($sql, $args)
      ->fetchAll();
    
    $rversion_prop = self::getTermByName('phenotype_r_compatible_version');
    $method_prop = self::getTermByName('phenotype_collection_method');
    
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
        WHERE type_id = :record_id 
          AND plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id = :project_id)
      ";
      $args = [':record_id' => $term_id, ':project_id' => $arr_properties['in_project_id']];

      $h = \Drupal::database() 
        ->query($sql, $args)
        ->fetchObject();

      $arr_properties['count_data'] = $h->data_count;

      // Count the projects this same column header is being used.
      $sql = "
        SELECT COUNT(project_id) AS project_count
        FROM pheno_project_cvterm 
        WHERE project_id <> :project_id AND cvterm_id = :record_id
      ";

      $h = \Drupal::database() 
        ->query($sql, $args)
        ->fetchObject();

      $arr_properties['count_project'] = $h->project_count;
    }

    return $arr_properties;
  }
}