<?php
/**
 * @file
 * Contains class definition of RawphenotypesEnvironment Data Service
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesEnvironmentDataService {
  /**
   * 
   */
  public static function saveEnvData($details) {
    $table = 'pheno_environment_data';
    // Create a row in the table.
    \Drupal::database()
      ->insert($table)
      ->fields([
        'project_id' => $details['project_id'],
        'fid'       => $details['fid'],
        'location' => $details['location'],
        'year'    => $details['year'],
        'sequence_no' => $details['sequence_no']
      ])
    ->execute();
  }

  /**
   * 
   */
  public static function getSequenceNumber($project_id, $location, $year) {
    // Rename file to include location, year, sequence no.
    // Fetch the largest seq no in a project, location and year.
    $sql = "
      SELECT sequence_no FROM pheno_environment_data
      WHERE location = :location AND year = :year AND project_id = :project_id
      ORDER BY sequence_no DESC LIMIT 1
    ";
    $args = [
      ':project_id' => $project_id,
      ':location' => $location,
      ':year'   => $year,
    ];

    $result = \Drupal::database()
      ->query($sql, $args);
    
    $result->allowRowCount = TRUE;
    
    return ($result->rowCount()) ? $result->fetchField() : 1;
  }
}