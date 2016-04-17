function data = read_plot_matrix(mySerial, method)

  if strcmp(method,'current') % compute the average error
    nsamples = fscanf(mySerial,'%d');       % first get the number of samples being sent
    data = zeros(nsamples,2);               % two values per sample:  ref and actual
    for i=1:nsamples
        data(i,:) = fscanf(mySerial,'%d %d'); % read in data from PIC32; assume ints, in mA
        times(i) = (i-1)*0.2;                 % 0.2 ms between samples
    end
    if nsamples > 1						        
        stairs(times,data(:,1:2));            % plot the reference and actual
    else
        fprintf('Only 1 sample received\n')
        disp(data);
    end
    score = mean(abs(data(:,1)-data(:,2)));
    fprintf('\nAverage error: %5.1f mA\n',score);
    title(sprintf('Average error: %5.1f mA',score));
    ylabel('Current (mA)');
    xlabel('Time (ms)'); 
  end
  
  if strcmp(method,'position') % compute the average error
    nsamples = fscanf(mySerial,'%d');       % first get the number of samples being sent
    data = zeros(nsamples,2);               % two values per sample:  ref and actual
    for i=1:nsamples
        data(i,:) = fscanf(mySerial,'%f %f'); % read in data from PIC32; assume ints, in mA
        times(i) = (i-1)*0.2;                 % 0.2 ms between samples
    end
    if nsamples > 1						        
        stairs(times,data(:,1:2));            % plot the reference and actual
    else
        fprintf('Only 1 sample received\n')
        disp(data);
    end
    score = mean(abs(data(:,1)-data(:,2)));
    fprintf('\nAverage error: %5.1f deg\n',score);
    title(sprintf('Average error: %5.1f deg',score));
    ylabel('Position (deg)');
    xlabel('Time (ms)'); 
  end
end
