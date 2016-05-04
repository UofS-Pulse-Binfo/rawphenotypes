/**
 * @file
 * Create graphical representation of phenotypic data.
 */
(function($) {
  Drupal.behaviors.rawphenoRawDataCreateHeatMap = {
    attach: function (context, settings) {
      ////
      // Leaf path/symbol used to mark a rep or when there is no data to chart.
      var leaf = 'm2.46906,17.20495c-0.42208,-1.86678 -3.37632,-8.39646 3.06637,-10.19122c6.44269,-1.79476 9.1144,-2.35212 11.17243,-3.20865c2.05803,-0.85653 5.22901,-3.1601 5.15685,-3.77696c-0.07214,-0.61685 1.40663,9.13151 -0.30335,14.13307c-1.70998,5.00156 -5.52874,7.53668 -10.36077,7.28237c-4.83199,-0.25432 -5.67973,-1.27159 -6.61222,-1.10203c-0.9325,0.16953 -1.56016,2.17883 -2.03454,3.39088c-0.47435,1.21206 -0.59338,1.35635 -0.50862,1.35635c0.08476,0 -1.78022,-1.01725 -2.03454,-1.35635c-0.25431,-0.33909 3.98429,-6.86654 8.98585,-8.64676c5.00161,-1.78023 7.19302,-4.28732 7.88385,-5.51021c0.69078,-1.22288 1.52588,-3.13656 1.52588,-3.17895c0,0.04239 -1.15433,3.08336 -3.6452,4.36577c-2.49086,1.2824 -7.79905,2.96701 -9.91833,4.66247c-2.11929,1.69544 -1.95157,3.647 -2.37363,1.78021l-0.00001,0z';
      
      // Tooltip/information box.
      var infoBox = d3.select('body')
        .append('div')   
        .attr('class', 'tool-tip')               
        .style('opacity', 0);
    
      // Div container of svg canvas.
      // The height and width of the chart canvas is based on the height and width of this
      // HTML element (DIV).
      var divChartContainer = d3.select('#container-rawdata');
      
      // Heatmap colour code.
      var color = ['#FFFFFF', '#E2EFDA', '#C6E0BA', '#A9D08E', '#70AD47', '#548235', '#375623'];
      
      // List of traits measured in a rep.
      var traitsList = [];
      
      var dataSet = {}, chartDimension = {}, rectDimension = {};
      
      // Canvas width and height;
      var width, height, margin = {};
  
      // Chart margins.
      margin.top = 40;
      margin.left = 90;
      margin.bottom = 80;
      margin.right = 30;
  
      // x axis scale.
      var x0, xAxis;
	   
	    // Height of the chart defined in the css rule for #container-rawdata.
      height = parseInt(divChartContainer.style('height'), 10);
      chartDimension.height = height - margin.top - margin.bottom; 
  
      // Main svg canvas of the heat map.  
      var svg;
  
      // Add event listener to window resize.
      // Reposition elements with the new window width.
      d3.select(window).on('resize', render);
  
      // Add event listener to trait selection box.
      // Add leaf symbol to reps.
      d3.select('#edit-select-trait').on('change', markRep);

	    // Barchart variables
	    // Main the svg canvas of the bar chart.
	    var bsvg;
	    var bx0, bxAxis, bins, dataByBins, dataByLocations, traitSelectedId, traitSelectedName;
	    // 10 px either side of a bin set (10X2)
      var gutter = 20;
      var barchartColor = d3.scale.category20b().domain([0, 20]);  
  
      // Option to clear the chart.
      $('#container-marker-information, #container-marker-information a').click(function(event) {
        event.preventDefault();
          
        // Close window.
        infoWindow('off');
        // Remove all markers.
        delMarker('');
        // Remove barchart.
        $('#container-barchart').remove();
      });
      
      
      // Start the heat map chart.
      // Read JSON data for heat map.
      var file = $('#rawdata-json').val();
	    d3.json(file, function(error, data) {
		    if (error) {
          // Error reading JSON.
          throw error;
        }
        else if(data == 0) {
          // No data.
          noData('.data-chart');
        }
        else {
          // Initialize heat map chart by adding all elements (rect, g, text, etc.) into the DOM,
          // and then call render function and position these element in the right place.
          initializeHeatmapChart(data);
        }
      });



      // Function to add all heat map chart elements into the DOM.
      // The height attribute or property of the elements remains constant whereas the
      // width is to be determined by the render function.
      function initializeHeatmapChart(data) {
        // Main svg canvas.
        svg = d3.select('.data-chart');
        svg.attr('height', height);
  
        // CHART DATA
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
            // This array associates an id (a rect) to list of traits the total count is based.
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
        heatmapChartInfo();
    
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
          .attr('transform', 'translate(' + margin.left + ',' + margin.top + ')');
          
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
            d3.select(this).style('opacity', 0.5);
            infoBox.transition().style('opacity', 1);
            infoBox
              .html('Rep ' + d.key + ': '+ d.values.count + ' Traits')  
              .style('left', (d3.event.pageX + 10) + 'px')     
              .style('top', (d3.event.pageY) + 'px');    
          })                  
          .on('mouseout', function(d) {
            d3.select(this).style('opacity', 1);  
            infoBox.transition().style('opacity', 0);   
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
          
        
        // Render heat map chart elements into the right place.
        render();
      }  
      // End initialize heat map chart.
      
      // Function to add all bar map chart elements into the DOM.
      // The height attribute or property of the elements remains constant whereas the
      // width is to be determined by the render function.
      function initializeBarChart(data) {
        // Add all barchart elements.
        bsvg = d3.select('.bar-chart')
          .attr('height', height - 30);
 
        var wrapper = bsvg.append('g')
          .attr('id', 'g-main-barchart-container')
          .attr('transform', 'translate(' + (margin.left + gutter) + ',' + margin.top + ')'); 

        // CHART DATA.
        // Locations / data.
        dataByLocations = d3.nest()
          .key(function(d) { return d.location; })
          .sortKeys(d3.ascending)
          .entries(data.data);         
   
        var locations = dataByLocations.map(function(d) { return d.key; });

        // Bins.
        bins = data.bin;

        // Count the number of rows per location in the same bin.
        // Bin / location / count stocks in the same bin.
        dataByBins = d3.nest()
          .key(function(d) { return d.bin; })
          .key(function(d) { return d.location; })
          .rollup(function(v) { 
            return v.length; 
          })
          .entries(data.data);
 
        var allCount = dataByBins.map(function(d) {  
          var c = d.values.map(function(d) { return d.values; });
          return c;
        });
        allCount = d3.merge(allCount);
   
        // CHART ELEMENTS.
        // Titles and legend
        barChartInfo();
      
        // X axis scale.
        bx0 = d3.scale.ordinal().domain(bins);
        bxAxis = d3.svg.axis()
          .orient('bottom')
          .scale(bx0);	

        // Y axis scale.
        // From the stock count, get the maximum.
        var maxCount = d3.max(allCount);
        var by0 = d3.scale.linear()
          .domain([0, maxCount])
          .range([chartDimension.height, 0]);

        var byAxis = d3.svg.axis().orient('left')
          .scale(by0);
        //       
    
       // Add grid.
       wrapper.selectAll('line.y')
         .data(by0.ticks())
         .enter()
         .append('line')
         .attr('class', 'grid-lines')
         .attr('class', 'y')
         .attr('x1', -20)
         .attr('y1', by0)
         .attr('y2', by0)
         .style('stroke', '#333333')
         .style('opacity', 0.1)
         .attr('shape-rendering', 'crispEdges')
         .attr('stroke-width', '1px');
    
        // Bar chart.
        // Add g container for each bin. Apply gutter/space to adjacent bins.
        // This will group elements into bins.
        wrapper.selectAll('g')
          .data(bins)
          .enter()
          .append('g')
            .attr('class', 'g-each-bin')
            .attr('id', function(d, i) {
              return 'bin-' + i;
            });
      
        // In each bin container, add same number of bars representing each location
        // present in the data. When a location is not present, add a space to maintain
        // equal order of location per bin.
        bins.forEach(function(bin, bin_i) {
          locations.forEach(function(location, location_i) { 
          
            var stockCount = +getStockCount(bin, location);
            var h = by0(stockCount);

            d3.select('#bin-' + bin_i)
              .append('g')
    
            .on('mousemove', function(d) {
              d3.select(this).style('opacity', 0.5);
              infoBox
                .transition()
                .style('opacity', 1);
              infoBox
                .html('Location ' + location +' ('+stockCount+' stocks)')  
                .style('left', (d3.event.pageX + 10) + 'px')     
                .style('top', (d3.event.pageY) + 'px');    
            })                  
            .on('mouseout', function(d) {       
              d3.select(this).style('opacity', 1);  
              infoBox.transition().style('opacity', 0);   
            })
    
            .append('rect')
            .attr('class', 'rect-each-bar')
              .attr('fill', barchartColor(location_i))
              .attr('y', h)
              .attr('height', (chartDimension.height - h));
          });
        });

        // X axis.
        bsvg.append('g')
          .attr('id', 'barchart-x-axis')
          .attr('class', 'barchart-axis')
          .attr('transform', 'translate(' + margin.left + ',' + (height - margin.bottom) + ')')
          .call(bxAxis);

        // Y axis.
        bsvg.append('g')
          .attr('class', 'barchart-axis')
          .attr('transform', 'translate(' + margin.left + ', ' + margin.top +')')
          .call(byAxis);	
        
        // Add chart title
        d3.select('#container-barchart')
          .append('h2')
			    .text('Figure: Distribution of ' + traitSelectedName + ' across Germplasm Phenotyped');
        
        // Position all bar chart elements into the right place.
        renderBarChart();
         
      }     
      // End initialize bar chart.



      // Render function to position chart elements in the svg canvas.
      // Function update stage with the current window width.
      function updateWidth() {
        // This is the width of the svg container.
        var containerWidth = divChartContainer.style('width');
        // Update the width to be used in redering the chart elements.
        width = parseInt(containerWidth, 10); 
      }
     
      // Function to render/position chart elements.
      function render() {
        // Get the width of the window before rendering chart elements.
        updateWidth();
       
        // Render heatmap.
        renderHeatmap();
       
        // Render barchart.
        // If barchart is present
		    if ($('#container-barchart').length > 0) {
		      renderBarChart();
		    }  
        
        // Remove all markers.
        delMarker('');
        // Close all windows.
        infoWindow('off');
        // Remove the bar chart.
        d3.selectAll('#container-barchart').remove();
      }
          
      // Render heat map chart elements.
      function renderHeatmap() {
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
          .duration(function() { return randomNumber(); })
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
      }
    
      // Render bar chart elements.
      function renderBarChart() {
        // Less 125px to accommodate legend.
        var legendArea = 125;
        
        // Adjust the width of svg canvas.
        bsvg.attr('width', width);
        
        var gBinWidth = Math.round((chartDimension.width - legendArea) / bins.length) - gutter;
        
        // render bins
        d3.selectAll('.g-each-bin')
          .attr('transform', function(d, i) {
            return 'translate('+ ((gBinWidth + gutter) * i) +', 0)';      
          });
    
        var barWidth = Math.round(gBinWidth / dataByLocations.length);
      
        var x = 0;
        d3.selectAll('.rect-each-bar')
          .style('opacity', 0)
          .transition()
          .duration(function() { return randomNumber(); })
          .ease('back')
          .style('opacity', 1)
          .attr('width', (barWidth - 2))
          .attr('x', function(d, i) {
            x = (i%dataByLocations.length == 0) ? 0 : x + 1;
            return x * barWidth; 
          });

        // Render scales
        bx0.rangeRoundBands([0, chartDimension.width - legendArea]);
        bxAxis.scale(bx0);
        d3.select('#barchart-x-axis').call(bxAxis);
        
        // Render Legend
        d3.select('#barchart-legend-rect')
          .attr('transform', 'translate('+ (width - margin.left - margin.right - 25) +', '+ margin.top +')');

        d3.select('#barchart-legend-text')
          .attr('transform', 'translate('+ (width - margin.left - margin.right - 25) +', '+ (margin.top + 10) +')');

        // Render x caption
        d3.select('#x-caption')
          .attr('transform', function() {
            return 'translate(' + Math.round(width/2) + ', '+ (height - margin.bottom + 40) +')';
          })
          
        // Render grids.
        d3.selectAll('line.y')
          .attr('x2', chartDimension.width - margin.right - margin.left - gutter);
      }



      // Label, caption, legend
      // Heat map
      function heatmapChartInfo() {
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
      
      // Bar chart.   
      function barChartInfo() {
        // Add legend.
        var legend = bsvg.append('g')
          .attr('id', 'barchart-legend-rect');
        
        var legendText = bsvg.append('g')
          .attr('id', 'barchart-legend-text');
          
        legend.selectAll('rect')
          .data(dataByLocations)
			    .enter()
			    .append('rect')
			    .attr('fill', function(d, i) { return barchartColor(i); })
			    .attr('height', 14)
			    .attr('width', 15)
			    .attr('x', 0)
			    .attr('y', function(d, i) { return 17 * i; })
	      
	      legendText.selectAll('text')
	        .data(dataByLocations)
	        .enter()
	        .append('text')
	        .text(function(d, i) { return d.key; })
          .attr('x', 19)
          .attr('class', 'legend-text')
			    .attr('y', function(d, i) { return 17 * i; });
      
        bsvg.append('g').append('text')
          .attr('class', 'chart-axes')
          .attr('id', 'y-caption')
          .attr('transform', function() {
            return 'translate(45, '+ Math.round(height/2) +') rotate(-90)';
          })
          .text('Number of Germplasm');   
       
        // Location - x axis.
        bsvg.append('g').append('text')
          .attr('id', 'x-caption')
          .attr('class', 'chart-axes')
          .text('Average [' + traitSelectedName + ']');
      }



      // Helper functions: 
      // Mark/hightlight reps and add informaton about the marker.
      function markRep() {
        d3.select('#text-not-found').remove();
        
        // Selected trait name.
		    traitSelectedName = $('#edit-select-trait option:selected').text();
		  
		    // Get selected trait. Trait select box returns cvterm id of a trait.
        traitSelectedId = $('#edit-select-trait').val();
        traitSelectedId.toString();
    
        if (traitSelectedId == '0') {
          // First option in the select box.
          // Remove all markers  
          delMarker('');
          // Add Information window.
		      infoWindow('off');
        }
        else {
          // Find trait selected in all rep and mark when present.
          var countRep = 0;
          for (var i = 1; i < traitsList.length; i++) {
            var id = '#obj-' + i.toString();
    
            if (traitsList[i].toString().indexOf(traitSelectedId) !== -1) {
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
              
            return traitSelectedName;
          });
        }
        else {
          infoWindow('off');
          if (traitSelectedId != '0') {
            $('.subtitle-left .form-item').append('<span id="text-not-found">&nbsp; Trait not found</span>');
          }
        }
       
       // Test if barchart is part of the DOM already.
        var containerBarchart = ($('#container-barchart').length > 0) ? 1 : 0;
        if (containerBarchart > 0 && traitSelectedId === '0') {
          // Remove barchart when it is present and 'select a trait' option is selected.
          d3.select('#container-barchart')
            .transition()
            .style('opacity', 0)
            .remove();  
        }
        else if (traitSelectedId !== '0') {
          // If trait selected is not equals to 'select a trait'.
          if (containerBarchart < 1) {
            // No barchart present in the DOM, Add chart elements.
            d3.select('.container-contents')
              .append('div')
                .attr('id', 'container-barchart')
                .transition()
                .style('opacity',1);
     
            d3.select('#container-barchart')
              .append('svg')
              .attr('class', 'bar-chart');
          }
          else {
            // Remove previous barchart g container elements.
            // Leave the svg canvas and append a new set of chart elements below.
            d3.selectAll('.bar-chart g, #container-barchart h2').remove();
          }

          // Render bar chart.
          var barchartFile = document.getElementById('rawdata-json-all').value + '?t=' + traitSelectedId;
          d3.json(barchartFile, function(error, barchartData) {
            if (error) {
              // Error reading JSON.
              throw error;
            }
            else if(barchartData == 0) {
              // No data.
              noData('.bar-chart');
            }
            else {
              initializeBarChart(barchartData);
            }
          });
        }
      } 
      
      // Show or hide the marker information window.
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
        if (id == '') {
          id = '.marker-rep';
        } 
        else {
          id = id + ' path';
        }
         
        if (d3.select(id).size() > 0) {
          d3.selectAll(id)
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
          
          d3.select(id)
            .append('path')
              .attr('class', 'marker-rep')
              .attr('d', leaf)
              .attr('transform', function() {
                // MARKER DIMENSION: 20X20 pixels
                // Location of rect plus half the bar width less the 1/2 width of the marker
                x = x + Math.round(rectDimension.width/2) - 10;
                y = y + Math.round(rectDimension.height/2) - 15;
                  
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
      // Return number as an index to a color element 
      // in color array.
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
     
      // Get the stock count of a location in a bin
      function getStockCount(bin, loc) {
        var d = dataByBins;
        for(var i = 0; i < d.length; i++) {
          if (d[i].key == bin) {
            for(var j = 0; j < d[i].values.length; j++) {
              var m = d3.values(d[i].values[j]);
              if (m[0] == loc) {
                return m[1];
              }
            }
            
            // Just search in the requested bin.
            return 0;  
          } 
        }
      }

      // Text and icon shown when there is no data.
      function noData(canvas) {
        var message = d3.select(canvas)
          .append('g')
          .attr('transform', 'translate(110, 50)');
          
        message
          .append('path')
          .attr('class', 'chart-no-data')
          .attr('title', 'No Data')
          .attr('d', leaf);
       
        message
          .append('text')
          .attr('class', 'chart-axes')
          .attr('y', 20)
          .attr('x', 50)
          .text('No data');
      }

      // Debugging function. Echo the contents of d.
      function echo(d) {
        //console.log(JSON.stringify(d));
        console.log(d);
      }
     ////
    }
  };
}(jQuery));