<?php
/**
 * @file
 * Contains class definition of RawphenotypesProjectService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesProjectService {
  /**
   * Get project profile by project name.
   * 
   * @param $project_name
   *   String, project name or title to search.
   * 
   * @return object
   *   Project row object.
   */
  public static function getProjectByName($project_name) {
    // project_id, name and description.
    $project_name = trim($project_name);

    $sql = "SELECT * FROM chado.project WHERE name = :project_name";
    $args = [':project_name' => $project_name];

    $project = \Drupal::database()
      ->query($sql, $args);
    
    $project->allowRowCount = TRUE;
    
    return ($project->rowCount()) ? $project->fetchObject() : null;
  }

  /**
   * Get project profile by project_id.
   * 
   * @param $project_id
   *   Integer, project id number.
   * 
   * @return object
   *   Project row object.
   */
  public static function getProject($project_id) {
    // project_id, name and description.
    $project_id = (int) trim($project_id);

    $sql = "SELECT * FROM chado.project WHERE project_id = :project_id";
    $args = [':project_id' => $project_id];

    $project = \Drupal::database()
      ->query($sql, $args);
    
    $project->allowRowCount = TRUE;
    
    return ($project->rowCount()) ? $project->fetchObject() : null;
  }

  /**
   * Add user to a project.
   * 
   * @param $user_id
   *   Integer, Drupal user user id.
   * @param $project_id
   *   Integer, project id number.
   */
  public static function addUserToProject($user_id, $project_id) {
    $table = 'pheno_project_user';

    $sql = sprintf("SELECT project_user_id FROM %s WHERE uid = :user AND project_id = :project", $table);
    $args = [':user' => $user_id, ':project' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    if ($query->rowCount() <= 0) { 
      // Only when user and project does not exist.
      \Drupal::service('database')
        ->insert($table)
        ->fields([
          'uid' => $user_id,
          'project_id' => $project_id
        ])
        ->execute();    
    }
  }

  /**
   * Get all projects that don't have column headers or new projects.
   * 
   * @return array
   *   Project rows from chado.project table.
   */
  public static function getNewProjects() {
    $sql = "
      SELECT t1.project_id, t1.name
      FROM chado.project AS t1 LEFT JOIN pheno_project_cvterm AS t2 USING(project_id)
      WHERE t2.project_id IS NULL ORDER BY t1.project_id DESC
    ";

    $project = \Drupal::database()
      ->query($sql);
    
    $project->allowRowCount = TRUE;
    
    return ($project->rowCount()) ? $project->fetchAllKeyed() : null;
  }

  /**
   * Get project assets (headers and users) count.
   * 
   * @return array
   */
  public static function getActiveProjects() {
    $sql = "
      SELECT
        t1.name,
        COUNT(DISTINCT t2.cvterm_id) AS header_count,
        COUNT(DISTINCT t3.cvterm_id) AS essential_count,
        COUNT(DISTINCT t4.uid) AS user_count,
        t1.project_id
      FROM
        chado.project AS t1
        INNER JOIN pheno_project_cvterm AS t2 USING (project_id)
        INNER JOIN pheno_project_cvterm AS t3 USING (project_id)
        INNER JOIN pheno_project_user AS t4 USING (project_id)
      WHERE
        t1.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
        AND t2.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
        AND t3.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
        AND t3.type = 'essential'
        AND t4.project_id IN (SELECT DISTINCT project_id FROM pheno_project_cvterm)
      GROUP BY t1.project_id
      ORDER BY t1.project_id DESC
    ";

    $project = \Drupal::database()
      ->query($sql);
    
    $project->allowRowCount = TRUE;
    
    return ($project->rowCount()) ? $project : [];
  }

  /**
   * Get all locations in a project.
   * 
   * @param $project_id
   *   Integer, project id number.
   * 
   * @return array
   *   All locations identified in a project.
   */
  public static function getProjectLocations($project_id) {
    $sql = "
      SELECT location
      FROM chado.rawpheno_rawdata_mview INNER JOIN pheno_plant_project USING(plant_id)
      WHERE project_id = :project_id
      GROUP BY location
      ORDER BY location ASC
    ";
    $args = [':project_id' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount() > 0) ? $query->fetchAllKeyed(0, 0) : [];
  }

  /**
   * Get all planting year in a project.
   * 
   * @param $project_id
   *   Integer, project id number.
   * @param $location
   *   String, a filter to return only years in a project 
   *   and location combination.
   * 
   * @return $array
   *   All years identified in a project.
   */
  public static function getProjectYears($project_id, $location = null) {
    $sql = "
      SELECT SUBSTRING(planting_date, 1, 4) AS year
      FROM chado.rawpheno_rawdata_mview
      WHERE location = :location
        AND plant_id IN (SELECT plant_id FROM pheno_plant_project WHERE project_id = :project_id)
      ORDER BY year DESC
    ";
    $args = [':project_id' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount() > 0) ? $query->fetchAllKeyed(0, 0) : [];
  }

  /**
   * Get column headers of a project.
   * 
   * @param $project_id
   *   Integer, project id number.
   * 
   * @return array
   *   An array of all column headers (cvterms and type) in a project.
   */
  public static function getProjectTerms($project_id) {
    $exclude = [
      'phenotype_plant_property_types',
      'phenotype_measurement_units',
      'phenotype_measurement_types',
      'phenotype_r_compatible_version',
      'phenotype_collection_method'
    ];

    $sql = "
      SELECT project_cvterm_id, pheno_project_cvterm.type
      FROM chado.cvterm RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
      WHERE project_id = :project_id AND name NOT IN(:exclude[])
      ORDER BY type, name ASC
    ";
    $args = [':project_id' => $project_id, ':exclude[]' => $exclude];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount() > 0) ? $query->fetchAll() : [];
  }
  
  /**
   * Get active user of a project.
   * 
   * @param $project_id
   *   Integer, project id number.
   * 
   * @return array
   *   Drupal user object - name, uid, mail, created, status and last login information.
   */
  public static function getProjectActiveUsers($project_id) {
    $sql = "
      SELECT project_user_id, uid FROM pheno_project_user
      WHERE project_id = :project_id
      ORDER BY uid DESC
    ";
    $args = [':project_id' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    $users = [];
    $cols = ['uid', 'name', 'mail', 'created', 'status', 'login'];
    if ($query->rowCount() > 0) {
      $user_service = \Drupal::service('rawphenotypes.user_service');

      foreach($query as $user) {
        $user_profile = $user_service::getUser((int)$user->uid, $cols);
        $users[ $user->project_user_id ] = $user_profile;
      }
    }

    return $users;
  }
  
  /**
   * Get project environment data files.
   * 
   * @param $project_id
   *   Integer, project id number.
   * 
   * @return array
   *   Environment data and file information (filename, uri and file timestamp).
   */
  public static function getProjectEnvDataFiles($project_id) {
    $sql = "
      SELECT environment_data_id, location, year, sequence_no, filename, uri, created as timestamp
      FROM pheno_environment_data INNER JOIN file_managed USING(fid)
      WHERE project_id = :project_id
      ORDER BY location, year, sequence_no ASC
    ";
    $args = [':project_id' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    return ($query->rowCount() > 0) ? $query->fetchAll() : [];
  }
}