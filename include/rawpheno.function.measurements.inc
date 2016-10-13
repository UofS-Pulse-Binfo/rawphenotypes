<?php
/**
 * @file
 * Master function to managed column headers.
 *
 * @params $type
 *   A string containing a description of column header set required.
 *
 * @return
 *   An array of column headers based on the type of set requested.
 */
function rawpheno_function_headers($type) {
  // List of traits/measurements from AGILE-PhenotypeDataCollection-v5.xlsx.
  // in AGILE project.
  // TRAIT/MEASUREMENT ---------------------------------------- INDEX
  // Order is the same order as in the spreadsheet file.
  // Note: Subset traits: 24, 25 and 26 are Hidden Column.
  //       Traits with Second trial eg. 1st; cm, 2nd; cm
  $arr_headers =
  array(
  'Plot',                                                       //0
  'Entry',                                                      //1
  'Name',                                                       //2
  'Rep',                                                        //3
  'Location',                                                   //4
  'Planting Date (date)',                                       //5
  '# of Seeds Planted (count)',                                 //6
  'Days to Emergence (days)',                                   //7
  '# of Emerged Plants (count)',                                //8
  'Days till 10% of Plants have Elongated Tendrils (days)',     //9
  'Days till 10% of Plants have One Open Flower (R1; days)',    //10
  '# Nodes on Primary Stem at R1 (1st; count)',                 //11
  '# Nodes on Primary Stem at R1 (2nd; count)',                 //12
  'Days till 10% of Plants have Pods (R3; days)',               //13
  'Days till 10% of Plants have fully Swollen Pods (R5; days)', //14
  'Days till 10% of Plants have 1/2 Pods Mature (R7; days)',    //15
  'R7 Traits: Lowest Pod Height (1st; cm)',                     //16
  'R7 Traits: Lowest Pod Height (2nd; cm)',                     //17
  'R7 Traits: Canopy Height (1st; cm)',                         //18
  'R7 Traits: Canopy Height (2nd; cm)',                         //19
  'Days till Harvest (days)',                                   //20
  'Diseases Present (y/n/?)',                                   //21
  'Disease-specific Comments',                                  //22
  'Lodging (Scale: 1-5) upright - lodged',                      //23
  'Subset Traits: # Peduncles (count)',                         //24
  'Subset Traits: # Pods (count)',                              //25
  'Subset Traits: # Seeds (count)',                             //26
  'Straw Biomass (g)',                                          //27
  'Total Seed Mass (g)',                                        //28
  'Total # of Seeds (count)',                                   //29
  '100 Seed Mass (g)',                                          //30
  'Comments'                                                    //31
  );

  // Determine the type of request.
  switch($type) {
    case 'phenotyping':
      // List of column headers used in standard phenotyping instructions page.
      // Used in: Instructions page.
      $type_id = array(5,7,8,9,10,11,14,15,16,18,20,21,23,24,25,26,27,28,29,30);
      break;

    case 'required':
      // List of required column headers - must have a value.
      // Used in: Upload Data - validate spreadsheet.
      $type_id = array(0,1,2,3,4);
      break;

    case 'expected':
      // List of column headers ids expected to be present in spreadsheet.
      // Used in: Upload Data - validate spreadsheet.
      //         .install file of this module.
      $type_id = array(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,
                       24,25,26,27,28,29,30,31);
      break;

    case 'plantprop':
      // List of required column headers excluding name - Plant Prop Traits.
      // Used in: Upload Data - Save spreadsheet.
      $type_id = array(0,1,3,4);
      break;

    case 'multi-trial':
      // List of column headers with first and secondary try.
      // Used in: .install file and Upload Data - save spreadsheet.
      $type_id = array(11,12,16,17,18,19);
      break;

    case 'essential':
      // List of essential column headers.
      // Used in: .install file.
      $type_id = array(0,1,2,3,4,5,6,7,8,9,10,15,20,21);
      break;

    case 'subset':
      $type_id = array(24,25,26);
      break;
  }

  $arr_requested_trait = array();
  // Create the array of traits based on requested trait ids.
  foreach($type_id as $id) {
    // Push the trait into the array.
    if ($type == 'phenotyping') {
      // Store the actual index number associated to a trait.
      // This is important since order is relevant in the instructions page.
      $arr_requested_trait[$id] = $arr_headers[$id];
    }
    else {
      // Order is irrelevant, just need the array of traits.
      array_push($arr_requested_trait, $arr_headers[$id]);
    }
  }

  return $arr_requested_trait;
}


/**
 * Function to transform a column header to R compatible version.
 *
 * @params $column_header
 *   A string containing the column header to transform.
 * @return
 *   A string containing the R compatible column header.
 */
function rawpheno_function_make_r_compatible($column_header) {
  // Get R transformation rules set in the admin control panel.
  $word_rules = variable_get('rawpheno_rtransform_words');
  $char_rules = variable_get('rawpheno_rtransform_characters');
  $replace_rules = variable_get('rawpheno_rtransform_replace');

  $arr_match = array();
  $arr_replace = array();
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
 * Function that lists types of column headers. This list will be available
 * as an option when adding traits. Plant Property is used to indicate
 * column headers in the pheno_plantprop.
 */
function rawpheno_function_trait_types() {
  return array('type1' => 'essential',
               'type2' => 'optional',
               'type3' => 'subset',
               'type4' => 'plantproperty');
}


/**
 * Function that lists trait reps or number of trials or R value/number
 * that precedes the unit of a measurement type.
 */
function rawpheno_function_trait_reps() {
  return array('1st', '2nd', '3rd', 'R1', 'R3', 'R5', 'R7');
}


/**
 * Function that list default/initial units available to this module.
 *
 * @param $set
 *   A string indicating the type of set to return.
 *   def - unit and definition.
 *   type - unit and data type.
 */
function rawpheno_function_default_unit($set) {
  // Type is required when programmatically generating data collection spreadsheet file
  // in instructions page.
  return array(
    'date'  => ($set == 'def') ? 'Date'  : 'date',
    'count' => ($set == 'def') ? 'Count' : 'integer',
    'days'  => ($set == 'def') ? 'Days'  : 'integer',
    'cm'    => ($set == 'def') ? 'Centimeters'  : 'integer',
    'scale' => ($set == 'def') ? 'Scale: 1-5'   : 'integer',
    'g'     => ($set == 'def') ? 'Grams (g)'    : 'integer',
    'text'  => ($set == 'def') ? 'Alphanumeric' : 'string',
    'y/n/?' => ($set == 'def') ? 'Yes, No or ? - Not sure' : 'string'
  );
}


/**
 * Function to test if the module has a project set up for user.
 *
 * @return FALSE when there is no project to work on.
 */
function rawpheno_function_project() {
  $sql = "SELECT FROM pheno_project_cvterm";
  $project_count = db_query($sql)
    ->rowCount();

  return ($project_count <= 0) ? 0 : 1;
}


/**
 * Function to test if the module has a project set up for user and
 * that project has dataset associated to it.
 *
 * @return FALSE when there is no data.
 */
function rawpheno_function_data() {
  $sql = "SELECT plant_project_id FROM pheno_plant_project";
  $stock_count = db_query($sql)
    ->rowCount();

  return ($stock_count <= 0) ? 0 : 1;
}


/**
 * Function to load projects appointed to user. In addition this function will
 * filter projects that don't have at least 1 essential trait.
 *
 * @param $user
 *   An integer containing the user id of the currently logged user.
 *
 * @return
 *   An array containing available projects.
 */
function rawpheno_function_user_project($user_id) {
  $trait_type = rawpheno_function_trait_types();
  $my_project = array();

  $sql = "SELECT
            t1.name,
            t1.project_id,
            COUNT(CASE WHEN t2.type = :essential_trait THEN 1 END) AS essential_count
          FROM
            {project} AS t1
            INNER JOIN pheno_project_cvterm AS t2 USING (project_id)
            LEFT JOIN pheno_project_user AS t3 USING (project_id)
          WHERE t3.uid = :user_id
          GROUP BY t1.project_id
          ORDER BY t1.project_id DESC";

  $args = array(':essential_trait' => $trait_type['type1'], ':user_id' => $user_id);
  $p = chado_query($sql, $args);

  if ($p->rowCount() > 0) {
    foreach($p as $a) {
      if ($a->essential_count > 0) {
        $my_project[$a->project_id] = $a->name;
      }
    }
  }

  return $my_project;
}


/**
 * Function create JSON summary data required in rawdata page.
 *
 * @param $project_id
 *   An integer containing the project id number.
 * @param $trait_id
 *   An integer containing the id number of a trait in a project.
 * @param $type
 *   A string indicating the type of data a given trait holds. It could be integer or text data type.
 * @param $data
 *   An array containing unique values in a given trait.
 *
 * @return
 *   A JSON summary data.
 *
 * NOTE: The following column headers are excluded from the traits that can be visualized.
 *   Planting Date (date),
 *   Disease-specific Comments,
 *   Comments
 *   # of Seeds Planted (count)
 */
function rawpheno_function_create_json($project_id, $trait_id, $type, $data = NULL) {
  // Array to hold the computed Bin.
  $arr_bin_json = array();
  // Array to hold the data.
  $arr_data_json = array();

  // NUMERIC DATA TYPE.
  if ($type == 'int') {
    // Get the average of each value and in a stock, in a location.
    $sql = "SELECT
              t1.stock_id AS stock,
              t3.value AS location,
              ROUND(AVG(t2.value::numeric)) AS average
            FROM
              {pheno_plant} AS t1
              INNER JOIN {pheno_measurements} AS t2 USING (plant_id)
              INNER JOIN {pheno_plantprop} AS t3 USING (plant_id)
            WHERE
              t2.type_id = :trait_id
              AND t3.type_id = (SELECT cvterm_id FROM chado.cvterm WHERE name = 'Location' LIMIT 1)
              AND t1.plant_id IN (SELECT plant_id FROM {pheno_plant_project} WHERE project_id = :project_id)
            GROUP BY t3.value, t1.stock_id, t2.value
            ORDER BY t3.value, average ASC";

    $args = array(':trait_id' => $trait_id, ':project_id' => $project_id);

    // Result set for averages.
    $d = db_query($sql, $args);

    // Get the min and max values from the averages.
    $all_average = $d->fetchCol(2);
    $max = max($all_average);
    $min = min($all_average);
    // Total number of bins.
    $total_bins = 4;
    // Size of each bin.
    $interval = round($max/$total_bins);

    $i = 0;
    $to = 0;
    $from = $min;

    while($to < $max) {
      $from = ($i == 0) ? $min : ($to + 1);
      $to = $from + $interval;

      $arr_bin_json[$i] = $from . '-' . $to;
      $i++;
    }

    // Assign each row with a bin.
    $m = db_query($sql, $args)
      ->fetchAll();

    foreach($m as $stock) {
      $stock_avg = (int)$stock->average;

      // Find the bin for this row.
      foreach($arr_bin_json as $b) {
        list($from, $to) = explode('-', $b);
        if ($stock_avg >= (int)$from AND $stock_avg <= (int)$to) {
          $stock_bin = $b;
          break;
        }
      }

      // Add entry to json
      $arr_data_json[] = array('location' => $stock->location,
                               'bin' => $stock_bin);
    }
  }
  // TEXT DATA TYPE
  elseif ($type == 'txt' OR $type == 'y/n/?') {
    // Text, list, enumeration of values.
    $arr_bin_json = ($type == 'txt') ? $data : array('y', 'n', '?');

    $sql = "SELECT
              t3.value AS location,
              t2.value AS v
            FROM
              {pheno_plant} AS t1
              INNER JOIN {pheno_measurements} AS t2 USING (plant_id)
              INNER JOIN {pheno_plantprop} AS t3 USING (plant_id)
            WHERE
              t2.type_id = :trait_id
              AND t3.type_id = (SELECT cvterm_id FROM chado.cvterm WHERE name = 'Location' LIMIT 1)
              AND t1.plant_id IN (SELECT plant_id FROM {pheno_plant_project} WHERE project_id = :project_id)
            ORDER BY t3.value ASC";

    $args = array(':trait_id' => $trait_id, ':project_id' => $project_id);

    // Result set for data/values.
    $d = db_query($sql, $args);

    foreach($d as $e) {
      // Find the bin for this data.
      foreach($arr_bin_json as $b) {
        if ($b == $e->v)
          $bin = $b;
      }

      $arr_data_json[] = ['location' => $e->location, 'bin' => $bin];
    }
  }
  // SCALE DATA TYPE
  elseif ($type == 'scale') {
    // Scale uses scale codes stored in scale memeber table.
    // Get the scale codes and make it the default bins. The scale codes are added in the install of this module.
    $s = db_query("SELECT code FROM {pheno_scale_member} ORDER BY code ASC");
    if ($s->rowCount() > 0) {
      foreach($s as $b) {
        $arr_bin_json[] = $b->code;
      }
    }
    else {
      // In case the scales are not available. Do not visualize.
      return 0;
    }

    $sql = "SELECT
              t3.value AS location,
              t2.value AS v
            FROM
              {pheno_plant} AS t1
              INNER JOIN {pheno_measurements} AS t2 USING (plant_id)
              INNER JOIN {pheno_plantprop} AS t3 USING (plant_id)
            WHERE
              t2.type_id = :trait_id
              AND t3.type_id = (SELECT cvterm_id FROM chado.cvterm WHERE name = 'Location' LIMIT 1)
              AND t1.plant_id IN (SELECT plant_id FROM {pheno_plant_project} WHERE project_id = :project_id)
            ORDER BY t3.value ASC";

    $args = array(':trait_id' => $trait_id, ':project_id' => $project_id);

    // Result set for data/values.
    $d = db_query($sql, $args);

    if ($d->rowCount() > 0) {
      foreach($d as $e) {
        $arr_data_json[] = ['location' => $e->location, 'bin' => $e->v];
      }
    }
  }


  // Return bin and data.
  return ['bin' => $arr_bin_json, 'data' => $arr_data_json];
}


/**
 * Function get cvterm definitions.
 *
 * @param $cvterm
 *   A string containing the cvterm name.
 *
 * @return
 *   A string containing the cvterm name definition.
 */
function rawpheno_function_get_definition($cvterm) {
  // Array to hold definitions.
  $definition = array();

  $method = 'method';
  $define = 'define';

$definition['Planting Date (date)'] = array(
$method =>
'Record the date the seeds were sown.',
$define =>
'The date should be the same for all plots, but could be different if circumstances such as bad weather prevent the seeding of all plots on the same day. If such a situation does occur, highlight rows with a different planting date so it is obvious to the data recorder, since they will have different days after planting values to record for that particular date.'
);

$definition['Days to Emergence (days)'] = array(
$method =>
'Record the number of days after planting for which 10% of seeds have emerged.',
$define =>
'Emergence = seedling stem/leaves have become visible.'
);

$definition['# of Emerged Plants (count)'] = array(
$method =>
'Record the number of plants which emerged.
When: Record values once plants begin to flower or have elongated tendrils.',
$define =>
'Emergence = seedling stem/leaves have become visible.'
);

$definition['Days till 10% of Plants have Elongated Tendrils (days)'] = array(
$method =>
'Record the number of days after planting for which 10% of plants have an elongated tendril.
Some plants may not produce elongated tendrils but develop a rudimentary tendril only 2-3 mm long. If this applies to more than 90% of plants in the plot, the "Days till 10% have Elongated Tendril" should be left blank.',
$define =>
'Elongated tendril = 5 mm and longer.'
);

$definition['Days till 10% of Plants have One Open Flower (R1; days)'] = array(
$method =>
'Record the number of days after planting for which 10% of plants have at least one open flower.',
$define =>
'Open flower = flower banner (standard petal) is visible.
R1 = One open flower at any node.'
);

$definition['# Nodes on Primary Stem at R1 (1st; count)'] = array(
$method =>
'Record the number of nodes on the primary stem when the first flower opens.
Record values from 2 plants, taken from the middle of the plot.',
$define =>
'Node = positions on stem where leaves and buds/branches grow from.
The first few nodes can loose their leaves and may not be readily visible. First flower may NOT be on the primary stem but we want the # of nodes on the primary stem that day.'
);

$definition['# Nodes on Primary Stem at R1 (2nd; count)'] = array(
$method =>
'Record the number of nodes on the primary stem when the first flower opens.
Record values from 2 plants, taken from the middle of the plot.',
$define =>
'Node = positions on stem where leaves and buds/branches grow from.
The first few nodes can loose their leaves and may not be readily visible. First flower may NOT be on the primary stem but we want the # of nodes on the primary stem that day.'
);

$definition['Days till 10% of Plants have Pods (R3; days)'] = array(
$method =>
'Record the number of days after planting for which 10% of plants have pods. Note: Pods can be present but still covered with flower petals, for ease of data collection, only count the plant as having a pod if you can visually see the pod without having to remove flower petals.',
$define => ''
);

$definition['Days till 10% of Plants have fully Swollen Pods (R5; days)'] = array(
$method =>
'Record the number of days after planting for which 10% of plants have pods with fully swollen seeds (that fill more than half of the pod area).',
$define =>
'Plant with pods = pods are visible without having to remove flower petals.
Swollen Pod = seeds have swollen to their max size and fill more than half the pod area.
R5 = Seed in any single pod on nodes 10-13 of the basal primary branch are swollen and completely fill the pod cavity.
Genotypic variation in seed size and pod structure will require the use of discretion by the data recorder, since not all genotypes have seeds which fully fill the pod cavity at maturity.
This corresponds to physiological maturity at which point the seeds have swollen to their max size. At this stage, seed coat is formed and there is a colour change in the cotyledons (except QG1!).'
);

$definition['Days till 10% of Plants have 1/2 Pods Mature (R7; days)'] = array(
$method =>
'Record the number of days after planting for which 10% of plants have 1/2 of their pods mature.',
$define =>
'Mature pod = dry pod ready to be harvested
Before the pods dry out they lose their green pigmentation, often looking pale, but will still contain moisture, which you can feel when you touch the pod. Pods that are considered mature will have changed colour and be dry to the touch.
R7 = The leaves start yellowing and 50% of the pods have turned yellow.
Pod maturity is not always accompanied by a yellowing of the pod – some pods turn white, some are pigmented and may have patterns, CDC QG2 will remain green'
);

$definition['R7 Traits: Lowest Pod Height (1st; cm)'] = array(
$method =>
'Record the distance (cm) from the soil to the bottom of the lower most pod.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.',
$define => ''
);

$definition['R7 Traits: Lowest Pod Height (2nd; cm)'] = array(
$method =>
'Record the distance (cm) from the soil to the bottom of the lower most pod.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.',
$define => ''
);

$definition['R7 Traits: Canopy Height (1st; cm)'] = array(
$method =>
'Record the distance (cm) from the soil to the highest part of the plant canopy.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
DO NOT stretch the plant. Leave as is.',
$define => ''
);

$definition['R7 Traits: Canopy Height (2nd; cm)'] = array(
$method =>
'Record the distance (cm) from the soil to the highest part of the plant canopy.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
DO NOT stretch the plant. Leave as is.',
$define => ''
);

$definition['Days till Harvest (days)'] = array(
$method =>
'Record the number of days from planting to harvest.',
$define => ''
);

$definition['Diseases Present (y/n/?)'] = array(
$method =>
'Record the presence of any disease, and if able, describe or make notes.',
$define =>
'Scale: y = disease present, n = no disease present ? = unsure
There is a "Disease-specific Comments" column for making any notes related to disease including but not limited to the observation that many or specific diseases are present.'
);

$definition['Disease-specific Comments'] = array(
$method =>
'Feel free to mention if multiple or specific diseases are present. Note: disease ratings for specific diseases should go in a separate column if you would like to measure them.',
$define => ''
);

$definition['Lodging (Scale: 1-5) upright - lodged'] = array(
$method =>
'Record the degree of plant lodging.
Scale:
1 = vertical/upright
2 = leaning
3 = most plants at 45° angle
4 = all plants 10-45° from ground
5 = most plants flat/prostrate
When: Record value when harvesting the plot.',
$define =>
'lodged = plant canopy is no longer vertical to the ground.'
);

$definition['Subset Traits: # Peduncles (count)'] = array(
$method =>
'Leave this column as is, DO NOT make any changes (unless you were unable to obtain 20 peduncles).
This has been preset to 20, because that is how many should be collected.',
$define =>
'peduncle = a stalk supporting an inflorescence (group/cluster of flowers).'
);

$definition['Subset Traits: # Pods (count)'] = array(
$method =>
'Record the total number of pods on the 20 peduncles collected for the subset traits.',
$define => ''
);

$definition['Subset Traits: # Seeds (count)'] = array(
$method =>
'Record the total number of seeds from pods counted for the previous trait ("Subset Traits: # Pods").',
$define => ''
);

$definition['Straw Biomass (g)'] = array(
$method =>
'Record the mass (g) of dry, above ground plant material from each plot.',
$define =>
'Straw = all above ground biomass excluding the seed.'
);

$definition['Total Seed Mass (g)'] = array(
$method =>
'Record the total mass (g) of all seeds harvested from each plot.',
$define => ''
);

$definition['Total # of Seeds (count)'] = array(
$method =>
'Record the total number of seeds harvested from each plot.',
$define => ''
);

$definition['100 Seed Mass (g)'] = array(
$method =>
'Count 100 seeds and record the mass (g).
Do not calculate this value from "Total Seed Mass" and "Total Number of Seeds".',
$define => ''
);

$definition['Comments'] = array(
$method => '',
$define => ''
);

$definition['R7 Traits: Canopy Width (cm)'] = array(
$method =>
'Record the max canopy width (cm).
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
Add a column with the header "R7 Traits: Canopy Width (cm)" if you would like to record this trait.',
$define => ''
);

$definition['R7 Traits: Plant Length (cm)'] = array(
$method =>
'Record the distance (cm) from the soil to the end of the longest stem.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
DO stretch the plant.
Add a column with the header "R7 Traits: Plant Length (cm)" if you would like to record this trait.',
$define => ''
);


  return (isset($definition[$cvterm])) ? $definition[$cvterm] : null;
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
function rawpheno_function_header_unit($header) {
  preg_match("/.*\(([^)]*)\)/", $header, $match);
  $u = (isset($match[1])) ? $match[1] : 'text';

  $chars = rawpheno_function_trait_reps();
  array_push($chars, ';', ': 1-5');

  $unit = str_ireplace($chars, '', $u);

  return trim(strtolower($unit));
}


/**
 * Function to get all properties of a column header.
 *
 * @param $asset_id
 *   An integer containing record id number of a given column header.
 * @param $dataset
 *   An string indicating whether to include data count.
 *
 * @return
 *   An array containing all properties (project, name, data, etc.) of a column header.
 */
function rawpheno_function_header_properties($asset_id, $dataset = NULL) {
  // Array to hold properties.
  $arr_properties = array();

  // Get project information and header type.
  $sql = "SELECT t1.project_id, t1.name, t2.cvterm_id, t2.type
          FROM {project} AS t1 INNER JOIN pheno_project_cvterm AS t2 USING(project_id)
          WHERE t2.project_cvterm_id = :record_id LIMIT 1";

  $args = array(':record_id' => $asset_id);
  $h = chado_query($sql, $args)
    ->fetchObject();

  $arr_properties['in_project_id'] = $h->project_id;
  $arr_properties['in_project_name'] = $h->name;
  $arr_properties['cvterm_id'] = $h->cvterm_id;
  $arr_properties['type'] = $h->type;

  // cvterm properties.
  $sql = "SELECT * FROM {cvterm} WHERE cvterm_id = :cvterm_id LIMIT 1";
  $args = array(':cvterm_id' => $arr_properties['cvterm_id']);

  $h = chado_query($sql, $args)
    ->fetchObject();

  $arr_properties['name'] = $h->name;
  $arr_properties['definition'] = empty($h->definition) ? '' : $h->definition;

  // cvterm R Version and Collection Method.
  $arr_properties['method'] = '';

  $h = chado_select_record('cvtermprop', array('value', 'type_id'), array('cvterm_id' => $arr_properties['cvterm_id']));
  $r_version = tripal_get_cv(array('name' => 'phenotype_r_compatible_version'));
  foreach($h as $c) {
    $val = isset($c->value) ? $c->value : '';

    if ($c->type_id == $r_version->cv_id) {
      $arr_properties['r_version'] = $val;
    }
    else {
      $arr_properties['method'] = $val;
    }
  }

  $args = array(':project_id' => $arr_properties['in_project_id'], ':cvterm_id' => $arr_properties['cvterm_id']);

  // Request full dataset including basic stats about the header.
  if ($dataset == 'full') {
    // Count data associated to column header.
    $sql = "SELECT COUNT(type_id) AS data_count FROM {pheno_measurements}
            WHERE type_id = :cvterm_id AND plant_id IN (SELECT plant_id FROM {pheno_plant_project} WHERE project_id = :project_id)";

    $h = db_query($sql, $args)
      ->fetchObject();

    $arr_properties['count_data'] = $h->data_count;

    // Count the projects this same column header is being used.
    $sql = "SELECT COUNT(project_id) AS project_count
            FROM {pheno_project_cvterm} WHERE project_id <> :project_id AND cvterm_id = :cvterm_id";

    $h = db_query($sql, $args)
      ->fetchObject();

    $arr_properties['count_project'] = $h->project_count;
  }

  return $arr_properties;
}
