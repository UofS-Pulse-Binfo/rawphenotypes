<?php
namespace Tests\install;

use StatonLab\TripalTestSuite\DBTransaction;
use StatonLab\TripalTestSuite\TripalTestCase;

class InstallTest extends TripalTestCase {
  // Uncomment to auto start and rollback db transactions per test method.
  use DBTransaction;

  /**
   * Test that the database tables created on install are there.
   *
   * Tables we are checking include:
   *  - pheno_plant
   *  - pheno_plantprop
   *  - pheno_scale_member
   *  - pheno_measurements
   *  - pheno_project_cvterm
   *  - pheno_plant_project
   *  - pheno_project_user
   *  - pheno_backup_file
   */
  public function testDBTablesExist() {

    $tables = array(
      'pheno_plant',
      'pheno_plantprop',
      'pheno_scale_member',
      'pheno_measurements',
      'pheno_project_cvterm',
      'pheno_plant_project',
      'pheno_project_user',
      'pheno_backup_file',
    );
    
    foreach($tables as $table) {
      $exists = db_table_exists($table);
      $this->assertTrue($exists, "Checking that $table table exists.");
    }

  }
}
