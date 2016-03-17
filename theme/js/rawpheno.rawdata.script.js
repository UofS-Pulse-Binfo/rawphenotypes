/**
 * @file
 * Create graphical representation of phenotypic data.
 */
(function($) {
  Drupal.behaviors.rawphenoRawData = {
    attach: function (context, settings) {
      startColour = 'green';
      // Colour map chart.
      var color = d3.scale.linear()
        .domain([0, 5, 10, 15, 20, 25, 30])
        .range(['#EAEAEA', '#E2EFDA', '#C6E0BA', '#A9D08E', '#70AD47', '#548235', '#375623']);

      //Margins and dimension.
      var height = 460;
      var width = 850;
      
      var halfHeight = Math.round(height / 2);
      var halfWidth = Math.round(width / 2);
      
      var barBorder = 1;
      var margin = {top: 40, right: 0, bottom: 90, left: 65};
      
      // Infobox container.
      var infoBox = d3.select('body')
        .append('div')   
        .attr('class', 'tool-tip')               
        .style('opacity', 0);
        
      // Main SVG canvas.
      var svg = d3.select('.data-chart');
      var chart = svg
        .attr('height', height)
        .attr('width', width)
        .append('g')
          .attr('id', 'g-locations')
          .attr("transform", "translate(" + margin.left + "," + margin.top + ")");
      
      // Standard title.
      var captions = svg.append('g').attr('id', 'g-captions');
      captions.append('text')
        .attr('class', 'chart-title')
        .attr('transform', function() {
          return 'translate('+ halfWidth +', 15)';
        })
        .text('Number of Trials per Location');
      
      // Growing season - y axis
      captions.append('text')
        .attr('class', 'chart-axes')
        .attr('transform', function() {
          return 'translate(10, '+ halfHeight +') rotate(-90)';
        })
        .text('Growing Season (year)');
      
      // Location - x axis
      captions.append('text')
        .attr('class', 'chart-axes')
        .attr('transform', function() {
          return 'translate('+ halfWidth +', '+ (height - margin.top) +')';
        })
        .text('Locations');
      
      // Legend
      var legend = svg.append('g')
        .attr('transform', 'translate(' + margin.left + ', '+ ((height - margin.top) + 20) +')')
        .attr('id', 'g-legend');
    
      legend.append('text')
        .attr('class', 'legend-text')
        .text('Number of traits')
        .attr('y', -5);        

      var j = 0;
      for(var m = 0; m <= 30; m = m+5) {
        legend.append('rect')
          .attr('transform', 'translate('+ (31 * j) +', 0)')
          .attr('width', 30)
          .attr('height', 7)
          .attr('fill', color(m));
          
        legend.append('text')
          .attr('class', 'legend-text')
          .attr('x', (30 * j) + 20)
          .attr('text-anchor', 'right')
          .attr('y', 17)
          .attr('width', 20)
          .text(m);  
        j++;  
      }
      //
      
      // json file
      var file = $('#rawdata-json').val();
      d3.json(file, function(error, data) {
      ///
        if (error) throw error;
        else if(data == 0) {
          chart.append('g')
            .on('mousemove', function(d) {
              d3.select(this).style('opacity', 0.5);
              infoBox
                .transition()        
                .style('opacity', 1);
              infoBox.html('No Data')  
                .style('left', (d3.event.pageX + 10) + 'px')     
                .style('top', (d3.event.pageY) + 'px');    
            })                  
            .on('mouseout', function(d) {       
              d3.select(this)
                .transition() 
                .delay(200)
                .style('opacity', 1);
              infoBox.transition()        
                .style('opacity', 0);   
            })
            .append('rect')
            .attr('fill', '#EAEAEA')
            .attr('width', '90%')
            .attr('height', '70%');
        }
        else {
          // Group data with location as primary key.
          // year as secondary key and rep as tertiary.
          // data /location/year/rep - values.
          var dataByLocation = d3.nest()
            .key(function(d) { return d.location; })
            .key(function(d) { return d.year; })
            .sortKeys(d3.descending)
            .key(function(d) { return d.rep; })
            .sortKeys(d3.ascending)
            .rollup(function(v) { 
              return d3.sum(v, function(d) { return d.trait; });
            })
            .entries(data);  
        
          var dataByRep = d3.nest()
            .key(function(d) { return d.rep; })
            .entries(data);
        
          var dataByYear = d3.nest()
            .key(function(d) { return d.year })
            .sortKeys(d3.descending)
            .entries(data);  
            
          // Compute dimensions based on number of records to chart.
          var numberOfLocation = dataByLocation.length;
          var numberOfRep = dataByRep.length;
          var numberOfYear = dataByYear.length;
          
          // Width of the svg less left margin.
          var chartHeight = height - margin.top - margin.bottom;
          var chartWidth = width - margin.left;
          
          // Container for each location.
          var locationContainerHeight = chartHeight;
          var locationContainerWidth = Math.round(chartWidth / numberOfLocation);
          
          // Each rect representing rep
          var barHeight = Math.round(locationContainerHeight / numberOfYear);
          var barWidth = Math.round(locationContainerWidth / numberOfRep);
          
          // X axis (locations)
          var x0 = d3.scale.ordinal()
            .rangeRoundBands([0, chartWidth]);
          var xAxis = d3.svg.axis()
            .scale(x0)
            .orient('bottom');
      
          // Y axis (year)
          var y0 = d3.scale.ordinal()
            .rangeRoundBands([0, chartHeight]);
          var yAxis = d3.svg.axis()
            .scale(y0)
            .orient('left');

          // Container g for each location.
          // Index to data: /location - read primary keys
          var g = chart.selectAll('g')
            .data(dataByLocation)
            .enter()
            .append('g')
              .attr('id', function(d, i) { return 'loc' + d.key; })
              .attr('transform', function(d, i) {
                return 'translate('+ (i * locationContainerWidth) +', 0)';      
              });
            
          // Container g for each year.
          // Index to data: /location/year - read primary key then secondary key.
          var y = g.selectAll('g')
            .data(function(d) { return d.values })
            .enter()
            .append('g')
            .attr('id', function(d) { return 'year' + d.key; })
            .attr('transform', function(d, i) {
              return 'translate(0, '+ (i * barHeight) +')';
            });
          
          y.selectAll('rect')
            .data(function(d) { return d.values; })
            .enter()
            .append('g')
            .on('mousemove', function(d) {
              d3.select(this)
                .style('opacity', 0.5);
              infoBox
                .transition()
                .style('opacity', 1);
              infoBox
                .html('Rep ' + d.key + ': '+ d.values + ' Traits')  
                .style('left', (d3.event.pageX + 10) + 'px')     
                .style('top', (d3.event.pageY) + 'px');    
            })                  
            .on('mouseout', function(d) {       
              d3.select(this)
                .transition() 
                .delay(200)
                .style('opacity', 1);  
              infoBox.transition()        
                .style('opacity', 0);   
            })
            .append('rect')
            .attr('stroke', '#FFFFFF')
            .attr('stroke-width', barBorder)
            .attr('fill', startColour)
            .transition()
            .duration(function(d, i) { return 500 * i; })
            .ease('back')
            .attr('fill', function(d) { return color(parseInt(d.values)); })
            .attr('width', barWidth)
            .attr('height', barHeight)
            .attr('x', function(d, i) { return i * barWidth; })
            .attr('y', 0)
            
          // Wrap each location in rect with border.
          g.append('rect')
            .attr('stroke', '#000000')
            .attr('stroke-width', (barBorder + 1))
            .attr('fill', 'none')
            .attr('width', locationContainerWidth)
            .attr('height', barHeight * numberOfYear);
        
          // Add x axis (location).
          x0.domain(dataByLocation.map(function(d) { return d.key; }));
          chart.append('g')
            .attr('class', 'axis')
            .attr('transform', 'translate(0,' + barHeight * numberOfYear +')')
            .call(xAxis);  
          
          // Add y axis (year).
          y0.domain(dataByYear.map(function(d) { return d.key; }));
          chart.append('g')
            .attr('class', 'axis')
            .attr('transform', 'translate(1,0)')
            .call(yAxis);  
        }
      ///
      });
    }
  };
}(jQuery));
