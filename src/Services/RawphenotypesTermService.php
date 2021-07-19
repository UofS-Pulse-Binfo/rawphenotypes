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

    \Drupal::service('database')
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
    
    $query = \Drupal::service('database')
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
}