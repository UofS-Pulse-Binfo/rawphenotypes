<?php
/**
 * @file
 * Contains class definition of RawphenotypesProjectService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesProjectService {
  /**
   * Get project.
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
  public static function getProjectProfile() {
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
}