<?php

/**
 * @file
 * Displays column header and instructions.
 * 
 * This page is the Standard Phenotyping Procedure. A page describing
 * what SOP for each trait and providing spreadsheet template file.
 *
 * Available variables:
 * - $path: The directory path to rawpheno module.
 * - $traits: Is an array containing column headers that requires phenotyping instructions.
 * - $form['search']: Is the form search field.
 * - $form['json_url']: Is a hidden field containing the url to menu callback. 
 */

// Directory path to rawpheno module.
$path = drupal_get_path('module', 'rawpheno');
// Use array to match traits.
$traits = rawpheno_function_headers('phenotyping');
// Index number and Trait equivalent
// 0  - 'Planting Date (date)'
// 1  - 'Days to Emergence (days)'
// 2  - '# of Emerged Plants (count)'
// 3  - 'Days till 10% of Plants have Elongated Tendrils (days)'
// 4  - 'Days till 10% of Plants have One Open Flower (R1; days)'
// 5  - '# Nodes on Primary Stem at R1 (1st; count)'
// 6  - 'Days till 10% of Plants have Pods (R3; days)'
// 7  - 'Days till 10% of Plants have fully Swollen Pods (R5; days)'
// 8  - 'Days till 10% of Plants have 1/2 Pods Mature (R7; days)'
// 9  - 'R7 Traits: Lowest Pod Height (1st; cm)'
// 10 - 'R7 Traits: Canopy Height (1st; cm)'
// 11 - 'Days till Harvest (days)'
// 12 - 'Diseases Present (y/n/?)'
// 13 - 'Lodging (Scale: 1-5) upright - lodged'
// 14 - 'Subset Traits: # Peduncles (count)'
// 15 - 'Subset Traits: # Pods (count)'
// 16 - 'Subset Traits: # Seeds (count)'
// 17 - 'Straw Biomass (g)'
// 18 - 'Total Seed Mass (g)'
// 19 - 'Total # of Seeds (count)'
// 20 - '100 Seed Mass (g)'
?>

<div id="phenotype-page" class="copy-center-row">
  <div id="container-option">
    <fieldset>
      <div class="fieldset-wrapper">
        <div class="tools">
          <?php print drupal_render($form["search"]); ?>
          <?php print drupal_render($form["json_url"]); ?>
        </div>  
  
        <div class="tools">
          &nbsp;<input type="button" id="btn_submit" name="btn_submit" class="form-submit" value="Search">
        </div>
      </div>
    </fieldset>
  </div>
  
  <div id="tabs" style="clear: both;">
    <ul>
      <li><a href="#fragment-1">Standard Procedure</a></li>
      <li id="essential"><a href="#fragment-2">Essential Traits</a></li>
      <li><a href="#fragment-3">Optional Traits</a></li>
      <li><a href="#fragment-4">Subset Traits</a></li>
      <li><a href="#fragment-5">Photo Appendix</a></li>
    </ul>
    
    <div id="fragment-1">
      <div class="container-link"> 
        <div class="download-template-instructions"><em>The following excel document should be used as a template for collecting phenotypic data:</em></div>
        <a href="#" class="button">Download</a> Download AGILE - Phenotype Data Collection (AGILE-PhenotypeDataCollection-v5.xlsx)
      </div> 
        
      <h3>Spreadsheet Tips</h3> 
      <ul> 
        <li><p>Comments with instructions, tips and scales are included with the header for each trait. Simply click the speech bubble icon beside the header.</p></li>
        <li><p>Essential Traits have green headers; all other traits are optional.</p></li> 
        <li><p>You should hide optional traits you are not taking data for by long pressing a column header then selecting “hide”.</p></li> 
        <li><p>We’ve included an easy calculator to determine “Days from Planting” in a separate tab.</p></li> 
        <li><p>Special Interest Traits: Add a column ot the spreadsheet for any other trait you are interested in collecting data for. When uploading you will be asked to provide a description including units or scale used to take the measurement.</p></li> 
      </ul> 
      <p> 
        These data will then be submitted through KnowPulse:<br />
        <a href="http://www.knowpulse.usask.ca/phenotypes/raw/upload">knowpulse.usask.ca/phenotypes/raw/upload</a>
      </p>  
    </div>
  
    <div id="fragment-2">
      <h3>These traits are essential to the AGILE project and data should be collected for all genotypes sent to you.</h3> 
      <table> 
        <tr><th width="180px">Trait</th> <th width="38%">Instructions</th> <th>Notes</th></tr>
        
        <?php if(isset($traits[0]) && !empty($traits[0])) { ?>
        <tr>
          <td><div class="data-cells"><?php print($traits[0]); //0 - Planting Date (date) ?></div></td> 
          <td>Record the date the seeds were sown.</td>
          <td>The date should be the same for all plots, but could be different if circumstances such as bad weather prevent the seeding of all plots on the same day. If such a situation does occur, <em>highlight rows with a different planting date</em> so it is obvious to the data recorder, since they will have different days after planting values to record for that particular date.</td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[1]) && !empty($traits[1])) { ?>
        <tr>
          <td><div class="data-cells"><?php print($traits[1]); //1 - Days to Emergence (days) ?></div></td> 
          <td>Record the number of days after planting for which 10% of seeds have emerged.</td>
          <td><strong>Definitions:</strong><br />* Emergence = seedling stem/leaves have become visible.</td>
        </tr>
        <?php } ?>
          
        <?php if(isset($traits[2]) && !empty($traits[2])) { ?>
        <tr>
          <td><div class="data-cells"><?php print($traits[2]); //2 - # of Emerged Plants (count) ?></div></td> 
          <td>Record the number of plants which emerged.</td>
          <td><strong>When:</strong> Record values once plants begin to flower or have elongated tendrils.</td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[3]) && !empty($traits[3])) { ?>  
        <tr>
          <td><div class="data-cells"><?php print($traits[3]); //3 - Days till 10% of Plants have Elongated Tendrils (days) ?></div></td> 
          <td>Record the number of days after planting for which 10% of plants have an elongated tendril.</td>
          <td>Some plants may not produce elongated tendrils but develop a rudimentary tendril only 2-3 mm long. If this applies to more than 90% of plants in the plot, the “Days till 10% have Elongated Tendril” should be left blank.<br />
          <strong>Definitions: </strong><br />* Elongated tendril = 5 mm and longer.</td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[4]) && !empty($traits[4])) { ?>  
        <tr>
          <td><div class="data-cells"><?php print($traits[4]); //4 - Days till 10% of Plants have One Open Flower (R1; days) ?></div></td> 
          <td>Record the number of days after planting for which 10% of plants have at least one open flower.</td>
          <td>Some plants may not produce elongated tendrils but develop a rudimentary tendril only 2-3 mm long. If this applies to more than 90% of plants in the plot, the “Days till 10% have Elongated Tendril” should be left blank.<br />
          <strong>Definitions: </strong><br />* Open flower = flower banner (standard petal) is visible.<br />
          * R1 = One open flower at any node.</td>
        </tr>          
        <?php } ?>
        
        <?php if(isset($traits[6]) && !empty($traits[6])) { ?>    
        <tr>
          <td><div class="data-cells"><?php print($traits[6]); //6 - Days till 10% of Plants have Pods (R3; days) ?></div></td> 
          <td>Record the number of days after planting for which 10% of plants have pods.</td>
          <td>Pods can be present but still covered with flower petals, for ease of data collection, only count the plant as having a pod if you can visually see the pod without having to remove flower petals.<br />
          <strong>Definitions: </strong><br />* Plant with pods = pods are visible without having to remove flower petals. <br />
          * R3 = Pod on nodes 10-13 of the basal primary branch visible.</td>
        </tr>   
        <?php } ?>
        
        <?php if(isset($traits[7]) && !empty($traits[7])) { ?>      
        <tr>
          <td><div class="data-cells"><?php print($traits[7]); //7 - Days till 10% of Plants have fully Swollen Pods (R5; days)' ?></div></td> 
          <td>Record the number of days after planting for which 10% of plants have pods with fully swollen seeds (that fill more than half of the pod area).</td>
          <td>Pods can be present but still covered with flower petals, for ease of data collection, only count the plant as having a pod if you can visually see the pod without having to remove flower petals.<br />
          <strong>Definitions: </strong><br />*Plant with pods = pods are visible without having to remove flower petals. <br />
          * Swollen Pod = seeds have swollen to their max size and fill more than half the pod area.<br />
          * R5 = Seed in any single pod on nodes 10-13 of the basal primary branch are swollen and completely fill the pod cavity.<br />
          <ul> 
            <li>Genotypic variation in seed size and pod structure will require the use of discretion by the data recorder, since not all genotypes have seeds which fully fill the pod cavity at maturity.</li>
            <li>This corresponds to physiological maturity at which point the seeds have swollen to their max size. At this stage, seed coat is formed and there is a colour change in the cotyledons (except QG1!).</li>
          </ul>
          </td>
        </tr>             
        <?php } ?>
        
        <?php if(isset($traits[8]) && !empty($traits[8])) { ?>  
        <tr>
          <td><div class="data-cells"><?php print($traits[8]); //8 - Days till 10% of Plants have 1/2 Pods Mature (R7; days) ?></div></td> 
          <td>Record the number of days after planting for which 10% of plants have 1/2 of their pods mature.</td>
          <td>Pods can be present but still covered with flower petals, for ease of data collection, only count the plant as having a pod if you can visually see the pod without having to remove flower petals.<br />
          <strong>Definitions: </strong><br />* Mature pod = dry pod ready to be harvested
          <ul>
            <li>Before the pods dry out they lose their green pigmentation, often looking pale, but will still contain moisture, which you can feel when you touch the pod. Pods that are considered mature will have changed colour and be dry to the touch.</li>
          </ul>
          * R7 = The leaves start yellowing and 50% of the pods have turned yellow.
          <ul>
            <li>Pod maturity is not always accompanied by a yellowing of the pod – some pods turn white, some are pigmented and may have patterns, CDC QG2 will remain green </li>
          </ul>  
          </td>
        </tr> 
        <?php } ?>
        
        <?php if(isset($traits[11]) && !empty($traits[11])) { ?>  
        <tr>
          <td><div class="data-cells"><?php print($traits[11]); //11 - Days till Harvest (days) ?></div></td> 
          <td>Record the number of days from planting to harvest.</td>
          <td>-</td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[12]) && !empty($traits[12])) { ?>    
        <tr>
          <td><div class="data-cells"><?php print($traits[12]); //12 - Diseases Present (y/n/?) ?></div></td> 
          <td>Record the presence of any disease, and if able, describe or make notes.</td>
          <td>
            <ul>
              <li>y = disease present</li>
              <li>n = no disease present</li>
              <li>? = unsure</li>
            </ul>
            There is a “Disease-specific Comments” column for making any notes related to disease including but not limited to the observation that many or specific diseases are present.
          </td>
        </tr>
        <?php } ?>
      </table>
      
      <h3>Further reference for Reproductive stages:</h3>
      <em>Erskine et al. (1990) Stages of Development in Lentil. Experimental Agriculture. 26(3): 297-302.</em>
      <ul>
        <li><strong>R1 - First Bloom</strong><p>One open flower at any node</p></li>
        <li><strong>R3 - Early Pod</strong><p>Pod on nodes 10-13 of the basal primary branch visible</p></li>
        <li><strong>R5 - Full Seed</strong><p>Seeds in any single pod on nodes 10-13 of the basal primary branch are swollen and completely fill the pod cavity</p></li>
        <li><strong>R7 - Physiological Maturity</strong><p>The leaves start yellowing and 50% of the pods have turned yellow</p></li>
      </ul> 
    </div>
  
    <div id="fragment-3">
      <h3>These traits are optional.</h3>         
      <p>We will be taking them in our location and have thus provided our procedure in case you interested in taking these data in your location as well. <em>Feel free to record ANY data you are interested in (including traits not listed below –just add a column to the accompanying data spreadsheet for traits not listed below).</em></p>
      
      <table> 
        <tr><th width="180px">Trait</th> <th width="38%">Instructions</th> <th>Notes</th></tr>
        
        <?php if(isset($traits[5]) && !empty($traits[5])) { ?>  
        <tr>
          <td><div class="data-cells"><?php print($traits[5]); //5 - # Nodes on Primary Stem at R1 (1st; count) ?></div></td> 
          <td>Record the number of nodes on the primary stem when the first flower opens.</td>
          <td>Record values from 2 plants, taken from the middle of the plot.<br />
          <strong>Definitions:</strong> Node = positions on stem where leaves and buds/branches grow from.
          <ul>
            <li>The first few nodes can loose their leaves and may not be readily visible.  First flower may NOT be on the primary stem but we want the # of nodes on the primary stem that day.</li>
          </ul>
          </td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[9]) && !empty($traits[9])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[9]); //9 - R7 Traits: Lowest Pod Height (1st; cm) ?></div></td> 
          <td>Record the distance (cm) from the soil to the bottom of the lower most pod.</td>
          <td>Record values from 2 plants, taken from the middle of the plot.<br />
          <strong>When:</strong> Record values when 10% of plants have 1/2 pods mature (R7).
          <ul>
            <li>Record values from 2 plants, taken from the middle of the plot.</li>
          </ul>
          </td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[10]) && !empty($traits[10])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[10]); //10 - R7 Traits: Canopy Height (1st; cm) ?></div></td> 
          <td>Record the distance (cm) from the soil to the highest part of the plant canopy.</td>
          <td>Record values from 2 plants, taken from the middle of the plot.<br />
          <strong>When:</strong> Record values when 10% of plants have 1/2 pods mature (R7).
          <ul>
            <li>Record values from 2 plants, taken from the middle of the plot.</li>
            <li>DO NOT stretch the plant. Leave as is.</li>
          </ul>
          </td>
        </tr>
        <?php } ?>
        
        <tr>
          <td><div class="data-cells">R7 Traits: Canopy Width (cm)</div></td> 
          <td>Record the max canopy width (cm).</td>
          <td>Record values from 2 plants, taken from the middle of the plot.<br />
          <strong>When:</strong> Record values when 10% of plants have 1/2 pods mature (R7).
          <ul>
            <li>Record values from 2 plants, taken from the middle of the plot.</li>
            <li>Add a column with the header “R7 Traits: Canopy Width (cm)” if you would like to record this trait.</li>
          </ul>
          </td>
        </tr>        
        
        <tr>
          <td><div class="data-cells">R7 Traits: Plant Length (cm)</div></td> 
          <td>Record the distance (cm) from the soil to the end of the longest stem.</td>
          <td>Record values from 2 plants, taken from the middle of the plot.<br />
          <strong>When:</strong> Record values when 10% of plants have 1/2 pods mature (R7).
          <ul>
            <li>Record values from 2 plants, taken from the middle of the plot.</li>
            <li>DO stretch the plant.</li>
            <li>Add a column with the header “R7 Traits: Plant Length (cm)” if you would like to record this trait.</li>
          </ul>
          </td>
        </tr>
        
        <?php if(isset($traits[13]) && !empty($traits[13])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[13]); //13 - Lodging (Scale: 1-5) upright - lodged ?></div></td> 
          <td>Record the degree of plant lodging.</td>
          <td>
          <ul>
            <li>1 = vertical/upright</li>
            <li>2 = leaning</li>
            <li>3 = most plants at 45° angle</li>
            <li>4 = all plants 10-45° from ground</li>
            <li>5 = most plants flat/prostrate</li>
          </ul>
          <strong>When:</strong> Record value when harvesting the plot.<br />
          <strong>Definitions:</strong><br />lodged = plant canopy is no longer vertical to the ground.<br />
          </td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[17]) && !empty($traits[17])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[17]); //17 - Straw Biomass (g) ?></div></td> 
          <td>Record the mass (g) of dry, above ground plant material from each plot.</td>
          <td>
          <strong>Definitions:</strong><br />Straw = all above ground biomass excluding the seed.
          </td>
        </tr>
        <?php } ?>
        
        <?php if(isset($traits[18]) && !empty($traits[18])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[18]); //18 - Total Seed Mass (g) ?></div></td> 
          <td>Record the total mass (g) of all seeds harvested from each plot.</td>
          <td>-</td>
        </tr>        
        <?php } ?>
        
        <?php if(isset($traits[19]) && !empty($traits[19])) { ?>        
        <tr>
          <td><div class="data-cells"><?php print($traits[19]); //19 - Total # of Seeds (count) ;?></div></td> 
          <td>Record the total number of seeds harvested from each plot.</td>
          <td>-</td>
        </tr>  
        <?php } ?>
        
        <?php if(isset($traits[20]) && !empty($traits[20])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[20]); //20 - 100 Seed Mass (g) ?></div></td> 
          <td>Count 100 seeds and record the mass (g).</td>
          <td>Do not calculate this value from “Total Seed Mass” and “Total Number of Seeds”.</td>
        </tr> 
        <?php } ?>                                
      </table>     
    </div>
  
    <div id="fragment-4">
      <h3>The following traits require a fair amount of work and, as such, are completely optional. We will collect them in SK, if you are interested in these traits, please contact us to make sure we are collecting the same thing.</h3>
      <p>
        Note: These columns are hidden by default. If you would like to record this data, select column “W” and either right-click (computer) or long-press (tablet) the column header then select “Unhide”.
        For the following “Subset Traits”, select 2 plants from the middle of each plot and randomly collect 10 peduncles from each plant, ranging from the top to bottom, for a total of 20 peduncles. If 20 peduncles cannot be obtained, sample from a 3rd plant.
      </p>
      
      <table> 
        <tr><th width="180px">Trait</th> <th width="38%">Instructions</th> <th>Notes</th></tr>
        
        <?php if(isset($traits[14]) && !empty($traits[14])) { ?> 
        <tr>
          <td><div class="data-cells"><?php print($traits[14]); //14 - Subset Traits: # Peduncles (count) ?></div></td> 
          <td>Leave this column as is, DO NOT make any changes (unless you were unable to obtain 20 peduncles).</td>
          <td>This has been preset to 20, because that is how many should be collected.<br />
          <strong>Definitions:</strong><br />peduncle = a stalk supporting an inflorescence (group/cluster of flowers).
          </td>
        </tr>
        <?php } ?> 
        
        <?php if(isset($traits[15]) && !empty($traits[15])) { ?>   
        <tr>
          <td><div class="data-cells"><?php print($traits[15]); //15 - Subset Traits: # Pods (count) ?></div></td> 
          <td>Record the total number of pods on the 20 peduncles collected for the subset traits.</td>
          <td>-</td>
        </tr>
        <?php } ?> 
        
        <?php if(isset($traits[16]) && !empty($traits[16])) { ?>  
        <tr>
          <td><div class="data-cells"><?php print($traits[16]); //16 - Subset Traits: # Seeds (count) ?></div></td> 
          <td>Record the total number of seeds from pods counted for the previous trait (“Subset Traits: # Pods”). </td>
          <td>-</td>
        </tr>
        <?php } ?> 
      </table>    
    </div>

    <div id="fragment-5">
      <h3>Topic: <select>
        <option value="0">Tendrils</option>
        <option value="1">Pods</option>
      </select></h3>
      
      <div id="photo-container">
        <div id="gallery-container">   
          <div class="side-nav"><a href="javascript:void();"><</a></div>
          <div class="gallery-img">
            <input type="hidden" id="path" value="../../<?php echo $path; ?>/instructions/img/appendix/">
            <input type="hidden" id="cur-img" value="0">
            <img src="../../<?php echo $path; ?>/instructions/img/appendix/01-tendrils-no-elongation.jpg">
            <br /><em>No elongation</em>
          </div>
          <div class="side-nav"><a href="javascript:void();">></a></div>
          <div style="clear:both">&nbsp;</div>
        </div>
      </div>
    </div>
  </div>
</div>