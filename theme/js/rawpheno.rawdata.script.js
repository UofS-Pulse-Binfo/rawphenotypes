/**
 * @file
 * Create graphical representation of phenotypic data.
 */
(function($) {
  Drupal.behaviors.rawphenoRawDataCreateHeatMap = {
    attach: function (context, settings) {
      ////
       var leaf = 'm3.35153,23.35191c-0.57288,-2.53377 -4.58266,-11.39645 4.16196,-13.83246c8.74462,-2.43602 12.3709,-3.19252 15.16425,-4.35508c2.79335,-1.16256 7.09729,-4.28918 6.99936,-5.12644c-0.09792,-0.83725 1.90921,12.39413 -0.41174,19.18271c-2.32095,6.78858 -7.50412,10.22948 -14.06259,9.88431c-6.55843,-0.34518 -7.70905,-1.72591 -8.97472,-1.49578c-1.26567,0.23011 -2.1176,2.95731 -2.76146,4.60242c-0.64383,1.64513 -0.80539,1.84097 -0.69035,1.84097c0.11505,0 -2.41628,-1.38071 -2.76147,-1.84097c-0.34517,-0.46024 5.40785,-9.3199 12.19643,-11.73618c6.78864,-2.41629 9.76302,-5.81914 10.70068,-7.47897c0.93759,-1.6598 2.07106,-4.25723 2.07106,-4.31476c0,0.05754 -1.56676,4.18502 -4.9476,5.92563c-3.38083,1.74059 -10.58559,4.0271 -13.46208,6.32834c-2.8765,2.30121 -2.64885,4.95005 -3.22171,2.41626l-0.00002,0z';
  
       // Tooltip/information box.
       var infoBox = d3.select('body')
         .append('div')   
         .attr('class', 'tool-tip')               
         .style('opacity', 0);
    
       // Div container of svg canvas.
       var divChartContainer = d3.select('#container-rawdata');
  
       // Heatmap colour code.
       var color = ['#FFFFFF', '#E2EFDA', '#C6E0BA', '#A9D08E', '#70AD47', '#548235', '#375623'];
  
       // List of traits measured in a rep.
       var traitsList = [];
  
       var dataSet = {}, chartDimension = {}, rectDimension = {};
  
       // Canvas width and height;
       var width, height, margin = {};
  
       // x axis scale.
       var x0, xAxis;
    
       // Chart margins.
       margin.top = 40;
       margin.left = 90;
       margin.bottom = 80;
       margin.right = 10;
  
       // Height of the chart defined in the css rule for #container-rawdata.
       height = parseInt(divChartContainer.style('height'), 10);
       chartDimension.height = height - margin.top - margin.bottom; 
  
       // Main svg canvas.   
       var svg;
  
       // Add event listener to window resize.
       // Reposition elements with the new window width.
       d3.select(window).on('resize', render);
  
       // Add event listener to select traits.
       // Add leaf symbol to reps.
       d3.select('#edit-select-trait').on('change', markRep);
  
       // Read JSON.
       var file = $('#rawdata-json').val();
       d3.json(file, function(error, data) {
       if (error) {
           // Error reading JSON.
           throw error;
         }
       else if(data == 0) {
         // No data.
         noData();
       }
       else {
         // Initialize chart by adding all elements (rect, g, text).
         // Call render function and position these element in the right place.
         initializeChart(data);
       }
       });
  
  
       // Function to add all chart elements.
       function initializeChart(data) {
         // Main svg canvas.
         svg = d3.select('.data-chart');
         svg.attr('height', height);
  
         // DATA
         // Group data with location as primary key.
         // year as secondary key and rep as tertiary.
         // data /location/year/rep - values.
         var n = 0;
         dataByLocation = d3.nest()
           .key(function(d) { return d.location; })
           .key(function(d) { return d.year; })
           .sortKeys(d3.descending)
           .key(function(d) { return d.rep; })
           .sortKeys(d3.sort)
           .rollup(function(v) {
             // Splice the data array values specific to a particular location, year and rep.
             var objEntry = v[0];
              
             n++;
             // This array associates an id (a rect) to list of traits the total count is based
             traitsList[n] = objEntry.type_id;
              
             return {
               'count': objEntry.trait,
               'id': 'obj-' + n.toString()
             }
         })
         .entries(data);  
      
         // All year.
         var dataByYear = d3.nest()
           .key(function(d) { return d.year })
           .sortKeys(d3.descending)
           .entries(data);       

         // All reps.
         var dataByRep = d3.nest()
           .key(function(d) { return d.rep; })
           .entries(data);
      
         // Count number of location, year and rep.
         dataSet.countLocation = dataByLocation.length;
         dataSet.countYear = dataByYear.length;
         dataSet.countRep = dataByRep.length;
    
         // CHART ELEMENTS
         // Titles and legend
         chartInfo();
    
         // Scales
         // X axis (locations)
           x0 = d3.scale.ordinal();
           xAxis = d3.svg.axis().orient('bottom');
    
         // Y axis (year)
         var y0 = d3.scale.ordinal()
           .rangeRoundBands([0, chartDimension.height]);
         var yAxis = d3.svg.axis()
           .scale(y0)
           .orient('left');	
      
         // Main chart wrapper. Add g element on position 
         // x = margin.left and y = margin.top  
         var chartWrapper = svg.append('g')
           .attr('id', 'g-chart-wrapper')
           .attr("transform", "translate(" + margin.left + "," + margin.top + ")");  
    
         // Each location, add a g container to hold a set of reps and year.  
         var locationWrapper = chartWrapper
           .selectAll('g')
           .data(dataByLocation).enter()
           .append('g')
             .attr('class', 'g-each-location')
             .attr('id', function(d, i) { return 'location-' + i; }); 
    
         // Each year, add a g container to hold a set of reps in a year.
         rectDimension.height = Math.round(chartDimension.height/dataSet.countYear);
         var yearWrapper = locationWrapper.selectAll('g')
           .data(function(d) { return d.values; }).enter()
           .append('g')
             .attr('class', 'g-each-year')
             .attr('transform', function(d, i) {
               return 'translate(0, '+ (i * rectDimension.height) +')';
             });
    
         // In g year container, add the reps (rect).
         yearWrapper.selectAll('rect')
           .data(function(d) { return d.values; }).enter()
           .append('g')
           .attr('id', function(d) { return d.values.id; })
           .attr('class', 'g-each-rep')
      
           .on('mousemove', function(d) {
                   d3.select(this)
                     .style('opacity', 0.5);
                   infoBox
                     .transition()
                     .style('opacity', 1);
                   infoBox
                     .html('Rep ' + d.key + ': '+ d.values.count + ' Traits')  
                     .style('left', (d3.event.pageX + 10) + 'px')     
                     .style('top', (d3.event.pageY) + 'px');    
                 })                  
                 .on('mouseout', function(d) {       
                   d3.select(this)
                     .style('opacity', 1);  
                   infoBox.transition()        
                     .style('opacity', 0);   
                 })
      
           .append('rect')
           .attr('class', 'rect-each-rep')
           .attr('fill', function(d) { 
             var c = setColour(parseInt(d.values.count));
             return color[c]; 
           })
           .attr('height', rectDimension.height)
           .attr('y', 0);
    
         // With all reps in place, wrap in a rect with thick border to visually
         // group a location with its reps.
         locationWrapper.append('rect')
           .attr('class', 'rect-wrap-each-location')
           .attr('height', rectDimension.height * dataSet.countYear); 
    
         // Finally, add scales
         // x axis (location).
         x0.domain(dataByLocation.map(function(d) { return d.key; }));
         chartWrapper.append('g')
           .attr('id', 'g-x-axis')
           .attr('class', 'axis')
           .attr('transform', 'translate(0,' + rectDimension.height * dataSet.countYear +')');
            
         // y axis (year).
         y0.domain(dataByYear.map(function(d) { return d.key; }));
         chartWrapper.append('g')
           .attr('class', 'axis')
           .attr('transform', 'translate(1,0)')
           .call(yAxis);
    
    
         // Render chart elements.
         render();
       }
  
  
       // Function to render/position chart elements.
       function render() {
         // Get the width of the window before rendering chart elements.
         updateWidth();
  
         // Adjust the width of svg canvas.
         svg.attr('width', width);
  
         // The width of the main chart.
         chartDimension.width = width - margin.left - margin.right;
  
         // Compute width of chart elements.
         var gLocationWrapperWidth = Math.round(chartDimension.width/dataSet.countLocation);
         rectDimension.width = Math.round(gLocationWrapperWidth/dataSet.countRep);
  
         // Render g elements for each location.
         // Render rect elements for each rep and wrapper for each location. 
         d3.selectAll('.g-each-location')
           .attr('transform', function(d, i) {
             return 'translate('+ (i * gLocationWrapperWidth) +', 0)';      
           });
  
         d3.selectAll('.rect-wrap-each-location')
           .attr('width', gLocationWrapperWidth);
  
         var x = 0;
           d3.selectAll('.rect-each-rep')
             .style('opacity', 0)
           .transition()
           .duration(function(d, i) { return randomNumber(); })
           .ease('back')
           .style('opacity', 1)
           .attr('width', rectDimension.width)
           .attr('x', function(d, i) {
           x = (i%dataSet.countRep == 0) ? 0 : x + 1;
             return x * rectDimension.width; 
           });
  
         // Render chart title (keep center) and x axis.
         d3.select('#standard-chart-title')
           .attr('transform', function() {
             return 'translate('+ Math.round(width/2) +', 15)';
           });
  
         // Render locations title.
         d3.select('#location-title')
           .attr('transform', function() {
             return 'translate('+ Math.round(width/2) + ', ' + (height - margin.top) +')';
           });	
  
         // Render scales
         x0.rangeRoundBands([0, chartDimension.width]);
         xAxis.scale(x0);
         d3.select('#g-x-axis').call(xAxis);
  
         // On resize, remove all markers.
         delMarker('');
         infoWindow('off');
       }
  
       // Function update stage with the current window width.
       function updateWidth() {
         // This is the width of the svg container.
         var containerWidth = divChartContainer.style('width');
         // Update the width to be used in redering the chart elements.
         width = parseInt(containerWidth, 10); 
       }
  
       // Mark/hightlight reps and add informaton about the marker.
       function markRep() {
         // Get selected trait. Trait select box returns cvterm id of a trait.
         var traitSelected = $('#edit-select-trait').val();
         traitSelected.toString();
    
         if (traitSelected == '0') {
           // First option in the select box.
           // Remove all markers  
           delMarker('');
           infoWindow('off');
         }
         else {
           // Find trait selected in all rep and mark when present.
           var countRep = 0;
           for (var i = 1; i < traitsList.length; i++) {
             var id = '#obj-' + i.toString();
    
             if (traitsList[i].toString().indexOf(traitSelected) !== -1) {
               countRep++;
               // Trait present
               // Add marker if none is present.
               addMarker(id);
             }
             else {
               // Not here.
               // Remove maker if marker is present.
               delMarker(id);
             }
           }
         }
  
         // Clear chart
         $('#container-marker-information, #container-marker-information a').click(function(event) {
           event.preventDefault();
          
           // Close window.
           infoWindow('off');
           // Remove all markers.
           delMarker('');
         });
  
         // Add information window when marker is present.
         if (countRep > 0) {
           // Make the trait selected title of the info window.
           $('#title-pheno').text(function() {
             infoWindow();
             // Tell user how many reps a trait was measured.
             $('#text-rep').text(countRep + ' Rep');
             // Remove trait not found message, if present.
             if ($('#text-not-found').length > 0) {
               $('#text-not-found').remove();
             }
             
             return $('#edit-select-trait option:selected').text();
           })
         }
         else {
           infoWindow('off');
           if (traitSelected != '0') {
             $('.subtitle-left .form-item').append('<span id="text-not-found">&nbsp; Trait not found</span>');
           }
         }
       }
  
       // Open and close the marker information window.
       function infoWindow(state) {
         var window = $('#container-marker-information');
  
         if (state == 'off') {
         window.slideUp(300);
         }
         else {
         window.slideDown(300);
         }
       }
  
       // Remove marker.
       function delMarker(id) {
         if (d3.select(id + ' path').size() > 0) {
           d3.selectAll(id + ' path')
             .transition()
             .duration(function() { return randomNumber(); })
             .style('opacity', 0)
             .remove();  
         }
       }
  
       // Mark rep when trait selected is measured in this rep.
       function addMarker(id) {
         if (d3.select(id + ' path').size() <= 0) {
           var rect = d3.select(id + ' rect');
           var x = parseInt(rect.attr('x'));
           var y = parseInt(rect.attr('y'));
    
           d3.select(id).append('path')
             .attr('class', 'marker-rep')
             .attr('d', leaf)
             .attr('transform', function() { 
               // MARKER DIMENSION: 35X35 pixels
               // Location of rect plus half the bar width less the 1/2 width of the marker
               x = x + Math.round(rectDimension.width/2) - 17;
               y = y + Math.round(rectDimension.height/2) - 17;
                    
               return 'translate(' + x + ',' + y + ')';
             })
             .style('opacity', 0)
             .transition()
             .duration(function() { return randomNumber(); })
             .style('opacity', 1)
             .attr('filter', 'url(#dropshadow)');
         
         }
       }
  
       // Map trait count to colour code.
       function setColour(num) {
         if (num == 0)
         return 0;
       else if (num >= 1 && num <= 5)
         return 1;
       else if (num >= 6 && num <= 10)
         return 2;
       else if (num >= 11 && num <= 15)
         return 3;
       else if (num >= 16 && num <= 20)
         return 4;
       else if (num >= 21 && num <= 25)
         return 5;
       else if (num >= 26) 
         return 6;
       }

       // Generate random numbers.
       function randomNumber() {
         var min = 1;
         var max = 10;
    
         return (Math.random() * (max - min) + min ) * 100;
       }  
  
       // Add chart title, axis and legend.
       function chartInfo() {
         var chartTitle = 'Number of Trials per Location';
         var chartXTitle = 'Locations';
         var chartYTitle = 'Growing Season (year)';
         var chartLegendTitle = 'Number of traits';
      
         // Title
         var captions = svg.append('g').attr('id', 'g-captions');
         captions.append('text')
           .attr('id', 'standard-chart-title')
           .attr('class', 'chart-title')
           .text(chartTitle);

         // Growing season - y axis
         captions.append('text')
           .attr('class', 'chart-axes')
           .attr('transform', function() {
             return 'translate(35, '+ Math.round(height/2) +') rotate(-90)';
           })
           .text(chartYTitle);          
          
         // Location - x axis.
         captions.append('text')
           .attr('id', 'location-title')
           .attr('class', 'chart-axes')
           .text(chartXTitle);	
  
  
         // Legend.
         var legend = svg.append('g')
           .attr('id', 'g-legend')
           .attr('transform', 'translate(' + margin.left + ', '+ ((height - margin.top) + 20) +')');
    
         legend.append('text')
             .attr('class', 'legend-text')
             .text(chartLegendTitle)
             .attr('y', -5);        

         var j = 0;
         for(var m = 0; m < 7; m++) {
           legend.append('rect')
             .attr('transform', 'translate('+ (31 * j) +', 0)')
             .attr('width', 30)
             .attr('height', 9)
             .attr('fill', color[m]);
          
           legend.append('text')
             .attr('class', 'legend-text')
             .attr('x', (30 * j) + 20)
             .attr('y', 17)
             .text((m * 5));  
           j++;  
         }
       }
  
       // No data.
       function noData() {
         d3.select('.data-chart')
           .attr('class', 'chart-no-data')
           .append('path')
           .attr('title', 'No Data')
           .attr('d', 'm16.47536,114.79264c-2.81615,-12.45542 -22.52731,-56.02235 20.45925,-67.99721c42.98654,-11.97492 60.8125,-15.6937 74.54397,-21.40858c13.73147,-5.71488 34.88865,-21.08462 34.40725,-25.2004c-0.48135,-4.11573 9.38524,60.9267 -2.02402,94.29783c-11.40926,33.37109 -36.88853,50.28576 -69.12846,48.589c-32.23974,-1.69684 -37.89592,-8.48418 -44.11767,-7.35292c-6.22174,1.13116 -10.40964,14.53746 -13.5747,22.62445c-3.16492,8.08708 -3.95911,9.04977 -3.3936,9.04977c0.56556,0 -11.87789,-6.78723 -13.57476,-9.04977c-1.69677,-2.26244 26.58376,-45.8145 59.95484,-57.69238c33.37142,-11.87793 47.99279,-28.60557 52.60211,-36.76491c4.60897,-8.15919 10.18086,-20.92757 10.18086,-21.21038c0,0.28285 -7.70184,20.57261 -24.32127,29.12905c-16.61938,8.55634 -52.03633,19.7963 -66.17649,31.10867c-14.14022,11.31223 -13.02115,24.33331 -15.83719,11.87778l-0.0001,0z');
       }
     ////
    }
  };
}(jQuery));




 