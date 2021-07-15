<?php
/**
 * @file
 * Contains class definition of RawphenotypesMviewService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesMviewService {
  // The name of the materialized view.
  public $mv_name = 'rawpheno_rawdata_summary';
  // Module name.
  public $mv_module = 'rawphenotypes';
  // Materialized view comment.
  public $mv_comment = 'Materialized view used by rawpheno module to generate summary of traits per location, rep and year.';
  // SQL.
  // Create a summary of data in the following format:
  // Plant id, Location, Rep, Planting Date and Total traits count.
  // NOTE: trait count includes planting date.
  // NOTE: As a materialized view, this get's executed by chado query.
  public $mv_sql = "
    SELECT CAST(t1.plant_id AS numeric) AS plant_id,
      t2.value AS location,
      t3.value AS rep,
      t5.value AS planting_year,
      COUNT(DISTINCT t4.type_id) AS total_count,
      ARRAY_TO_STRING(ARRAY_AGG(DISTINCT t4.type_id), ',') AS all_traits
    FROM pheno_plant AS t1
      INNER JOIN [pheno_plantprop] AS t2 USING(plant_id)
      INNER JOIN [pheno_plantprop] AS t3 USING(plant_id)
      INNER JOIN [pheno_measurements] AS t4 USING(plant_id)
      INNER JOIN [pheno_measurements] AS t5 USING(plant_id)
    WHERE
      t2.type_id = (SELECT cvterm_id FROM {cvterm} cvt LEFT JOIN {cv} cv ON cv.cv_id=cvt.cv_id WHERE cvt.name = 'Location' AND cv.name = 'phenotype_plant_property_types') AND
      t3.type_id = (SELECT cvterm_id FROM {cvterm} cvt LEFT JOIN {cv} ON cv.cv_id=cvt.cv_id WHERE cvt.name = 'Rep' AND cv.name = 'phenotype_plant_property_types') AND
      t5.type_id = (SELECT cvterm_id FROM {cvterm} cvt LEFT JOIN {cv} ON cv.cv_id=cvt.cv_id WHERE cvt.name = 'Planting Date (date)' AND cv.name = 'phenotype_measurement_types')
    GROUP BY t1.plant_id, t4.plant_id, t2.value, t3.value, t5.value
  ";
  // Materialized view table name.
  public $mv_table = 'rawpheno_rawdata_mview';
  // Schema.
  public $mv_schema = [
    'table' => 'rawpheno_rawdata_mview',
    'fields' => [
      'plant_id' => ['type' => 'int'],
      'location' => [
        'type' => 'varchar',
        'length' => 255
      ],
      'rep' => [
        'type' => 'varchar',
        'length' => 255
      ],
      'planting_date' => [
        'type' => 'varchar',
        'length' => 255
      ],
      'total_count' => [
        'type' => 'int'
      ],
      'all_traits' => [
        'type' => 'text'
      ],
    ],
    'indexes' => [
      'plant_id' => ['plant_id'],
      'location' => ['location'],
    ]
  ];


  /**
   * Create materialized view required by this modules.
   * 
   * @TODO: Implement upgraded service relating to materialized views.
   */
  public static function createMview() {
    $mv_name = self::$mv_name;
    $mv_module = self::$mv_module;
    $mv_schema = self::$mv_schema;
    $mv_sql = self::$mv_sql;
    $mv_comment = self::$mv_comment;

    // Create materialized view.
    if (function_exists('chado_add_mview')) {
      chado_add_mview($mv_name, $mv_module, $mv_schema, $mv_sql, $mv_comment, FALSE);
    }
    else {
      // tripal_add_mview($mv_name, $mv_module, $mv_schema, $mv_sql, $mv_comment, FALSE);
      // CREATE MATERIALIZED VIEWS.
    }
  }

  /**
   * Drop materialized view.
   * 
   * @see uninstall hook.
   * @TODO: Implement upgraded service relating to materialized views.
   */
  public static function dropMview() {
    $schema = \Drupal::service('database')->schema();
    if ($schema->tableExists('tripal_mview')) {
      // Tripal maview table exists, check if it has a row
      // matching the materialized view this module is using.
      $sql = "SELECT mview_id FROM {tripal_mviews} WHERE mv_table = :mv_table";
      $args = [':mv_table' => self::$mv_table];
      
      $mview = \Drupal::database()
        ->query($sql, $args);
      
      if (isset($mview_id) AND $mview_id > 0) {
        // Delete materialized view.
        // DELETE MVIEW HERE
      }       
    }
  }
}