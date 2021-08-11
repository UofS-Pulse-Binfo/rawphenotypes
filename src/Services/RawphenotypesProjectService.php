<?php
/**
 * @file
 * Contains class definition of RawphenotypesProjectService.
 * This class will provide management of resources or assests per project.
 * 
 * Default project of this module is set to 
 * AGILE: Application of Genomic Innovation in Lentil Economy.
 * If this project is present in your system, a set of headers will be
 * assigned. This set is an AGILE-project-specific headers.
 * 
 * @see rawphenotypes\Services\RawphenotypesDefaultValueService
 */

namespace Drupal\Rawphenotypes\Services;

use Drupal\file\Entity\File;
use Drupal\Core\Url;

class RawphenotypesProjectService {
  /**
   * Get project project records project_id, name and description by
   * using project name as search key.
   * 
   * @param $project_name
   *   String, project name or title (search key).
   * 
   * @return object
   *   Project row object.
   */
  public static function getProjectByName($project_name) {
    // project_id, name and description.
    $project_name = trim($project_name);

    $sql = "SELECT * FROM chado.project WHERE name = :project_name LIMIT 1";
    $args = [':project_name' => $project_name];

    $project = \Drupal::database()
      ->query($sql, $args);
    
    return $project->fetchObject() ?? null;
  }

  /**
   * Get project project records project_id, name and description by
   * using project id number as search key.
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

    $sql = "SELECT * FROM chado.project WHERE project_id = :project_id LIMIT 1";
    $args = [':project_id' => $project_id];

    $project = \Drupal::database()
      ->query($sql, $args);
    
    return $project->fetchObject() ?? null;
  }

  /**
   * Delegate a user (registered Drupal user) to a project.
   * This will give user the priviledge to among others upload data.
   * 
   * In custom table pheno_project_user
   * - uid: Drupal user id.
   * - project_id: project the user is assigned to.
   * 
   * @param $user_id
   *   Integer, Drupal user user id.
   * @param $project_id
   *   Integer, project id number.
   */
  public static function addUserToProject($user_id, $project_id) {
    $sql = "
      SELECT project_user_id FROM pheno_project_user 
      WHERE uid = :user AND project_id = :project
    ";
    $args = [':user' => $user_id, ':project' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;

    if ($query->rowCount() <= 0) { 
      // Check to ensure that user is assigned to a project once.
      // User is not in this project yet.
      \Drupal::service('database')
        ->insert('pheno_project_user')
        ->fields([
          'uid' => $user_id,
          'project_id' => $project_id
        ])
        ->execute();    
    }
  }

  /**
   * Get a list of project from chado.project table.
   * The list will provide administrators a selection for
   * phenotyping experiment using this module.
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
   * Get all active project and provide a basic summary of project assets.
   * - Header count: number of column header a project is measuring.
   * - Essential header count: number of essential header (important header/ must exists).
   * - User count: number of user assigned to a project.
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
    
    return ($project->rowCount()) ? $project->fetchAll() : [];
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
   * Get all planting year project has carried an experiment and filtered
   * by a corresponding location.
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
   * Get all column header asset of a project.
   * 
   * @param $project_id
   *   Integer, project id number.
   * 
   * @return array
   *   An array of all column headers (cvterms and type) in a project.
   */
  public static function getProjectTerms($project_id) {
    $sql = "
      SELECT project_cvterm_id, pheno_project_cvterm.type
      FROM chado.cvterm RIGHT JOIN pheno_project_cvterm USING(cvterm_id)
      WHERE project_id = :project_id ORDER BY type, name ASC
    ";
    $args = [':project_id' => $project_id];

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
        $users[ $user->project_user_id ] = (object) $user_profile;
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
    $files = [];

    $sql = "
      SELECT environment_data_id, location, year, sequence_no, fid
      FROM pheno_environment_data
      WHERE project_id = :project_id
      ORDER BY location, year, sequence_no DESC
    ";
    $args = [':project_id' => $project_id];

    $query = \Drupal::database()
      ->query($sql, $args);
    
    $query->allowRowCount = TRUE;  

    if ($query->rowCount() > 0) {
      foreach($query as $i => $env) {
        $f = new \stdClass();
        $f->environment_data_id = $env->environment_data_id;
        $f->location = $env->location;
        $f->year = $env->year;
        $f->sequence_no = $env->sequence_no;

        $file_entity = File::load($env->fid);
        $f->filename = $file_entity->getFilename();
        $f->uri = file_create_url($file_entity->getFileUri());
        $timestamp = \Drupal::service('date.formatter')
          ->format($file_entity->getCreatedTime());
        $f->created = $timestamp;
        $f->filesize = format_size($file_entity->getSize())
          ->render();

        $files[ $i ] = $f;
      }
    } 
    
    return $files;
  }

  /**
   *  Get project environment data assets - year, location and file.
   * 
   * @param $asset_id
   *   Integer, environemnt data asset id. 
   * 
   * @return array
   *   Evironment data assets location, year and file.
   */
  public static function getProjectEnvDataAssets($asset_id) {
    $sql = "
      SELECT environment_data_id, project_id, location, year, fid
      FROM pheno_environment_data 
      WHERE environment_data_id = :envdata_id
      LIMIT 1
    ";
    $args = [':envdata_id' => $asset_id];
    $envdata = \Drupal::database()
      ->query($sql, $args);

    if ($envdata) {
      $e = $envdata->fetchObject();

      return array(
        'envdata_id' => $e->environment_data_id,
        'project_id' => $e->project_id,
        'location' => $e->location,
        'year' => $e->year,
        'fid' => $e->fid,
      );
    }
    else {
      return 0;
    }
  }
}