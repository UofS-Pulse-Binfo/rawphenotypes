<?php
/**
 * @file
 * Contains class definition of RawphenotypesDefaultValueService.
 */

namespace Drupal\Rawphenotypes\Services;

class RawphenotypesDefaultValueService {
  /**
   * Function to manage terms used by this module.
   *
   * @param $type
   *   Type of default terms/values required.
   * 
   * @return
   *   An array of string representing scale measurements, variable names,
   *   controlled vocabulary and measurement units.
   */
  public static function getDefaultValue($type) {
    $default = '';

    switch($type) {
      case 'project':
        $default = 'AGILE: Application of Genomic Innovation in the Lentil Economy';
        break;

      case 'defaults':
        // Default colour scheme, heading/title and R transformation rules.
        $default = [
          'colour' => '#304356',
          'rawdata' => 'Phenotypic data available',
          'download' => 'Select locations and traits that you want to download',
          'instructions' => 'Standard Phenotyping Procedure',
          'upload' => 'Drag and Drop phenotypic data collection spreadsheet',
          'backup' => 'My files',
          'r-words' => 'of,to,have,on,at',
          'r-chars' => '(,),/,-,:,;,%',
          'r-replace' => '# = num,/ = div,? = unsure,- = to',
        ];
        break;

      case 'scales':
        // Scale equivalent for scale units.
        $default = [
          'Lodging (Scale: 1-5) upright - lodged' => [
            1 => 'Vertical/upright',
            2 => 'Leaning',
            3 => 'Most plants at 45 degrees angle',
            4 => 'All plants 10-45 degrees from ground',
            5 => 'Most plants flat/prostrate',
          ]
        ];
        break;

      case 'variables':
        // Configuration variable names.
        $default = [
          'rawpheno_colour_scheme',
          'rawpheno_rawdata_title',
          'rawpheno_download_title',
          'rawpheno_instructions_title',
          'rawpheno_upload_title',
          'rawpheno_backup_title',
          'rawpheno_rtransform_words',
          'rawpheno_rtransform_characters',
          'rawpheno_rtransform_replace',
        ];
        break;

      case 'vocabularies':
        // Controlled vocabularies.
        $default = [
          // Use this cv to create relationships
          // ie. term-unit, unit-method.
          'cv_phenotypes' => 'rawphenotypes_terms',
          'cv_prop' => 'phenotype_plant_property_types',
          'cv_unit' => 'phenotype_measurement_units',
          'cv_type' => 'phenotype_measurement_types',
          'cv_rver' => 'phenotype_r_compatible_version',
          'cv_desc' => 'phenotype_collection_method'
        ];
        break;

      case 'units':
        // Units available in the spreadsheet.
        $default = [
          'date'  => 'Date',
          'count' => 'Count',
          'days'  => 'Days',
          'cm'    => 'Centimeters', 
          'scale' => 'Scale: 1-5',
          'g'     => 'Grams (g)',
          'text'  => 'Alphanumeric',
          'y/n/?' => 'Yes, No or ? - Not sure',
        ];
        break;
    }

    return $default;
  }

  /**
   * Default column headers used by default project.
   *
   * @params $type
   *   A string containing a description of column header set required.
   *
   * @return
   *   An array of column headers based on the type of set requested.
   */
  public static function getTraits($type) {
    // List of traits/measurements from AGILE-PhenotypeDataCollection-v5.xlsx.
    // in AGILE project.
    // TRAIT/MEASUREMENT ---------------------------------------- INDEX
    // Order is the same order as in the spreadsheet file.
    // Note: Subset traits: 24, 25 and 26 are Hidden Column.
    //       Traits with Second trial eg. 1st; cm, 2nd; cm
    $arr_headers = [
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
    ];

    // Determine the type of request.
    switch($type) {
      case 'phenotyping':
        // List of column headers used in standard phenotyping instructions page.
        // Used in: Instructions page.
        $type_id = [5, 7, 8, 9, 10, 11, 14, 15, 16, 18, 20, 21, 23, 24, 25, 26, 27, 28, 29, 30];
        break;

      case 'required':
        // List of required column headers - must have a value.
        // Used in: Upload Data - validate spreadsheet.
        $type_id = [0, 1, 2, 3, 4];
        break;

      case 'expected':
        // List of column headers ids expected to be present in spreadsheet.
        // Used in: Upload Data - validate spreadsheet.
        //         .install file of this module.
        $type_id = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 
                    22, 23, 24, 25, 26, 27, 28, 29, 30, 31];
        break;

      case 'plantprop':
        // List of required column headers excluding name - Plant Prop Traits.
        // Used in: Upload Data - Save spreadsheet.
        $type_id = [0, 1, 3, 4];
        break;

      case 'multi-trial':
        // List of column headers with first and secondary try.
        // Used in: .install file and Upload Data - save spreadsheet.
        $type_id = [11, 12, 16, 17, 18, 19];
        break;

      case 'essential':
        // List of essential column headers.
        // Used in: .install file.
        $type_id = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 21];
        break;

      case 'plot':
        // Plot column headers
        // Used in: upload file to determine if to re-use plant_id or insert a new row.
        // plot, rep, year (planting date), location.
        $type_id = [0, 3, 4, 5];
        break;

      case 'subset':
        $type_id = [24, 25, 26];
        break;
    }

    $arr_requested_trait = [];
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
   * Function that lists types of column headers. This list will be available
   * as an option when adding traits. Plant Property is used to indicate
   * column headers in the pheno_plantprop.
   */
  public static function getTraitTypes() {
    return [
      'type1' => 'essential',
      'type2' => 'optional',
      'type3' => 'subset',
      'type4' => 'plantproperty',
      'type5' => 'contributed'
    ];
  }

  /**
   * Function that lists trait reps or number of trials or R value/number
   * that precedes the unit of a measurement type.
   */
  public static function getTraitReps() {
    return ['1st', '2nd', '3rd', '4th', 'R1', 'R3', 'R5', 'R7'];
  }

  /**
   * Default terms/trait with definition and method information.
   *
   * @param $cvterm
   *   A string containing the cvterm name.
   *
   * @return
   *   A string containing the cvterm name definition.
   */
  public static function defineTrait($cvterm) {
    // Array to hold definitions.
    $definition = [];

    $method = 'method';
    $define = 'define';

$definition['Planting Date (date)'] = [
$method =>
'Record the date the seeds were sown.',
$define =>
'The date should be the same for all plots, but could be different if circumstances such as bad weather prevent the seeding of all plots on the same day. If such a situation does occur, highlight rows with a different planting date so it is obvious to the data recorder, since they will have different days after planting values to record for that particular date.'
];

$definition['Days to Emergence (days)'] = [
$method =>
'Record the number of days after planting for which 10% of seeds have emerged.',
$define =>
'Emergence = seedling stem/leaves have become visible.'
];

$definition['# of Emerged Plants (count)'] = [
$method =>
'Record the number of plants which emerged.
When: Record values once plants begin to flower or have elongated tendrils.',
$define =>
'Emergence = seedling stem/leaves have become visible.'
];

$definition['Days till 10% of Plants have Elongated Tendrils (days)'] = [
$method =>
'Record the number of days after planting for which 10% of plants have an elongated tendril.
Some plants may not produce elongated tendrils but develop a rudimentary tendril only 2-3 mm long. If this applies to more than 90% of plants in the plot, the "Days till 10% have Elongated Tendril" should be left blank.',
$define =>
'Elongated tendril = 5 mm and longer.'
];

$definition['Days till 10% of Plants have One Open Flower (R1; days)'] = [
$method =>
'Record the number of days after planting for which 10% of plants have at least one open flower.',
$define =>
'Open flower = flower banner (standard petal) is visible.
R1 = One open flower at any node.'
];

$definition['# Nodes on Primary Stem at R1 (1st; count)'] = [
$method =>
'Record the number of nodes on the primary stem when the first flower opens.
Record values from 2 plants, taken from the middle of the plot.',
$define =>
'Node = positions on stem where leaves and buds/branches grow from.
The first few nodes can loose their leaves and may not be readily visible. First flower may NOT be on the primary stem but we want the # of nodes on the primary stem that day.'
];

$definition['# Nodes on Primary Stem at R1 (2nd; count)'] = [
$method =>
'Record the number of nodes on the primary stem when the first flower opens.
Record values from 2 plants, taken from the middle of the plot.',
$define =>
'Node = positions on stem where leaves and buds/branches grow from.
The first few nodes can loose their leaves and may not be readily visible. First flower may NOT be on the primary stem but we want the # of nodes on the primary stem that day.'
];

$definition['Days till 10% of Plants have Pods (R3; days)'] = [
$method =>
'Record the number of days after planting for which 10% of plants have pods. Note: Pods can be present but still covered with flower petals, for ease of data collection, only count the plant as having a pod if you can visually see the pod without having to remove flower petals.',
$define => ''
];

$definition['Days till 10% of Plants have fully Swollen Pods (R5; days)'] = [
$method =>
'Record the number of days after planting for which 10% of plants have pods with fully swollen seeds (that fill more than half of the pod area).',
$define =>
'Plant with pods = pods are visible without having to remove flower petals.
Swollen Pod = seeds have swollen to their max size and fill more than half the pod area.
R5 = Seed in any single pod on nodes 10-13 of the basal primary branch are swollen and completely fill the pod cavity.
Genotypic variation in seed size and pod structure will require the use of discretion by the data recorder, since not all genotypes have seeds which fully fill the pod cavity at maturity.
This corresponds to physiological maturity at which point the seeds have swollen to their max size. At this stage, seed coat is formed and there is a colour change in the cotyledons (except QG1!).'
];

$definition['Days till 10% of Plants have 1/2 Pods Mature (R7; days)'] = [
$method =>
'Record the number of days after planting for which 10% of plants have 1/2 of their pods mature.',
$define =>
'Mature pod = dry pod ready to be harvested
Before the pods dry out they lose their green pigmentation, often looking pale, but will still contain moisture, which you can feel when you touch the pod. Pods that are considered mature will have changed colour and be dry to the touch.
R7 = The leaves start yellowing and 50% of the pods have turned yellow.
Pod maturity is not always accompanied by a yellowing of the pod – some pods turn white, some are pigmented and may have patterns, CDC QG2 will remain green'
];

$definition['R7 Traits: Lowest Pod Height (1st; cm)'] = [
$method =>
'Record the distance (cm) from the soil to the bottom of the lower most pod.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.',
$define => ''
];

$definition['R7 Traits: Lowest Pod Height (2nd; cm)'] = [
$method =>
'Record the distance (cm) from the soil to the bottom of the lower most pod.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.',
$define => ''
];

$definition['R7 Traits: Canopy Height (1st; cm)'] = [
$method =>
'Record the distance (cm) from the soil to the highest part of the plant canopy.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
DO NOT stretch the plant. Leave as is.',
$define => ''
];

$definition['R7 Traits: Canopy Height (2nd; cm)'] = [
$method =>
'Record the distance (cm) from the soil to the highest part of the plant canopy.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
DO NOT stretch the plant. Leave as is.',
$define => ''
];

$definition['Days till Harvest (days)'] = [
$method =>
'Record the number of days from planting to harvest.',
$define => ''
];

$definition['Diseases Present (y/n/?)'] = [
$method =>
'Record the presence of any disease, and if able, describe or make notes.',
$define =>
'Scale: y = disease present, n = no disease present ? = unsure
There is a "Disease-specific Comments" column for making any notes related to disease including but not limited to the observation that many or specific diseases are present.'
];

$definition['Disease-specific Comments'] = [
$method =>
'Feel free to mention if multiple or specific diseases are present. Note: disease ratings for specific diseases should go in a separate column if you would like to measure them.',
$define => ''
];

$definition['Lodging (Scale: 1-5) upright - lodged'] = [
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
];

$definition['Subset Traits: # Peduncles (count)'] = [
$method =>
'Leave this column as is, DO NOT make any changes (unless you were unable to obtain 20 peduncles).
This has been preset to 20, because that is how many should be collected.',
$define =>
'peduncle = a stalk supporting an inflorescence (group/cluster of flowers).'
];

$definition['Subset Traits: # Pods (count)'] = [
$method =>
'Record the total number of pods on the 20 peduncles collected for the subset traits.',
$define => ''
];

$definition['Subset Traits: # Seeds (count)'] = [
$method =>
'Record the total number of seeds from pods counted for the previous trait ("Subset Traits: # Pods").',
$define => ''
];

$definition['Straw Biomass (g)'] = [
$method =>
'Record the mass (g) of dry, above ground plant material from each plot.',
$define =>
'Straw = all above ground biomass excluding the seed.'
];

$definition['Total Seed Mass (g)'] = [
$method =>
'Record the total mass (g) of all seeds harvested from each plot.',
$define => ''
];

$definition['Total # of Seeds (count)'] = [
$method =>
'Record the total number of seeds harvested from each plot.',
$define => ''
];

$definition['100 Seed Mass (g)'] = [
$method =>
'Count 100 seeds and record the mass (g).
Do not calculate this value from "Total Seed Mass" and "Total Number of Seeds".',
$define => ''
];

$definition['Comments'] = [
$method => 'Feel free to mention any remarks or observations.',
$define => 'Comments'
];

$definition['R7 Traits: Canopy Width (cm)'] = [
$method =>
'Record the max canopy width (cm).
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
Add a column with the header "R7 Traits: Canopy Width (cm)" if you would like to record this trait.',
$define => ''
];

$definition['R7 Traits: Plant Length (cm)'] = [
$method =>
'Record the distance (cm) from the soil to the end of the longest stem.
Record values from 2 plants, taken from the middle of the plot.
When: Record values when 10% of plants have 1/2 pods mature (R7).
Record values from 2 plants, taken from the middle of the plot.
DO stretch the plant.
Add a column with the header "R7 Traits: Plant Length (cm)" if you would like to record this trait.',
$define => ''
];

    return (isset($definition[$cvterm])) ? $definition[$cvterm] : null;
  }
}