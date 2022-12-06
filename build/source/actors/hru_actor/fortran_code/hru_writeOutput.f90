module HRUwriteoOutput_module
USE,intrinsic :: iso_c_binding

USE data_types,only:&
                    var_i,          &  
                    var_i8,         &
                    var_d,          &
                    var_ilength,    &
                    var_dlength,    &
                    flagVec
! named variables to define new output files
USE netcdf
USE netcdf_util_module,only:netcdf_err 
USE nrtype
USE globalData,only:noNewFiles
USE globalData,only:newFileEveryOct1
USE globalData,only:chunkSize               ! size of chunks to write
USE globalData,only:outputPrecision         ! data structure for output precision
USE globalData,only:integerMissing            ! missing integer
! metadata
USE globalData,only:time_meta                 ! metadata on the model time
USE globalData,only:forc_meta                 ! metadata on the model forcing data
USE globalData,only:diag_meta                 ! metadata on the model diagnostic variables
USE globalData,only:prog_meta                 ! metadata on the model prognostic variables
USE globalData,only:flux_meta                 ! metadata on the model fluxes
USE globalData,only:indx_meta                 ! metadata on the model index variables
USE globalData,only:bvar_meta                 ! metadata on basin-average variables
USE globalData,only:bpar_meta                 ! basin parameter metadata structure
USE globalData,only:mpar_meta                 ! local parameter metadata structure
! child metadata for stats
USE globalData,only:statForc_meta             ! child metadata for stats
USE globalData,only:statProg_meta             ! child metadata for stats
USE globalData,only:statDiag_meta             ! child metadata for stats
USE globalData,only:statFlux_meta             ! child metadata for stats
USE globalData,only:statIndx_meta             ! child metadata for stats
USE globalData,only:statBvar_meta             ! child metadata for stats
! index of the child data structure
USE globalData,only:forcChild_map             ! index of the child data structure: stats forc
USE globalData,only:progChild_map             ! index of the child data structure: stats prog
USE globalData,only:diagChild_map             ! index of the child data structure: stats diag
USE globalData,only:fluxChild_map             ! index of the child data structure: stats flux
USE globalData,only:indxChild_map             ! index of the child data structure: stats indx
USE globalData,only:bvarChild_map             ! index of the child data structure: stats bvar
USE globalData,only:outFreq                   ! output frequencies
! named variables
USE var_lookup,only:iLookTIME                 ! named variables for time data structure
USE var_lookup,only:iLookDIAG                 ! named variables for local column model diagnostic variables
USE var_lookup,only:iLookPROG                 ! named variables for local column model prognostic variables
USE var_lookup,only:iLookINDEX                ! named variables for local column index variables
USE var_lookup,only:iLookFreq                 ! named variables for the frequency structure
USE get_ixname_module,only:get_freqName       ! get name of frequency from frequency index


implicit none
private
public::initStatisticsFlags
public::writeHRUToOutputStructure

contains
subroutine initStatisticsFlags(handle_statCounter, handle_outputTimeStep, &
    handle_resetStats, handle_finalizeStats, err) bind(C, name="initStatisticsFlags")
  USE var_lookup,only:maxvarFreq                ! maximum number of output files
  USE var_lookup,only:iLookFreq                 ! named variables for the frequency structure
  implicit none
  ! dummy variables
  type(c_ptr), intent(in), value       :: handle_statCounter
  type(c_ptr), intent(in), value       :: handle_outputTimeStep
  type(c_ptr), intent(in), value       :: handle_resetStats
  type(c_ptr), intent(in), value       :: handle_finalizeStats
  integer(c_int), intent(out)          :: err
  ! local variables
  type(var_i), pointer                 :: statCounter
  type(var_i), pointer                 :: outputTimeStep
  type(flagVec), pointer               :: resetStats
  type(flagVec), pointer               :: finalizeStats
  ! Convert C pointers to Fortran pointers
  call c_f_pointer(handle_statCounter, statCounter)
  call c_f_pointer(handle_outputTimeStep, outputTimeStep)
  call c_f_pointer(handle_resetStats, resetStats)
  call c_f_pointer(handle_finalizeStats, finalizeStats)
  ! Start of Subroutine

  ! initialize the statistics flags
  allocate(statCounter%var(maxVarFreq), stat=err)
  allocate(outputTimeStep%var(maxVarFreq), stat=err)
  statCounter%var(1:maxVarFreq) = 1
  outputTimeStep%var(1:maxVarFreq) = 1

  allocate(resetStats%dat(maxVarFreq), stat=err)
  allocate(finalizeStats%dat(maxVarFreq), stat=err)
  ! initialize flags to reset/finalize statistics
  resetStats%dat(:)    = .true.   ! start by resetting statistics
  finalizeStats%dat(:) = .false.  ! do not finalize stats on the first time step

  ! set stats flag for the timestep-level output
  finalizeStats%dat(iLookFreq%timestep)=.true.


end subroutine initStatisticsFlags

subroutine writeHRUToOutputStructure(&
                            indxHRU,                   &
                            indxGRU,                   &
                            outputStep,                & ! index into the output Struc
                            ! statistics variables
                            handle_forcStat,           & ! model forcing data
                            handle_progStat,           & ! model prognostic (state) variables
                            handle_diagStat,           & ! model diagnostic variables
                            handle_fluxStat,           & ! model fluxes
                            handle_indxStat,           & ! model indices
                            handle_bvarStat,           & ! basin-average variables
                            ! primary data structures (scalars)
                            handle_timeStruct,         & ! x%var(:)     -- model time data
                            handle_forcStruct,         & ! x%var(:)     -- model forcing data
                            ! primary data structures (variable length vectors)
                            handle_indxStruct,         & ! x%var(:)%dat -- model indices
                            handle_mparStruct,         & ! x%var(:)%dat -- model parameters
                            handle_progStruct,         & ! x%var(:)%dat -- model prognostic (state) variables
                            handle_diagStruct,         & ! x%var(:)%dat -- model diagnostic variables
                            handle_fluxStruct,         & ! x%var(:)%dat -- model fluxes
                            ! basin-average structures
                            handle_bparStruct,         & ! x%var(:)     -- basin-average parameters
                            handle_bvarStruct,         & ! x%var(:)%dat -- basin-average variables
                            ! local HRU data
                            handle_statCounter,        & ! x%var(:)
                            handle_outputTimeStep,     & ! x%var(:)
                            handle_resetStats,         & ! x%var(:)
                            handle_finalizeStats,      & ! x%var(:)
                            handle_finshTime,          & ! x%var(:)    -- end time for the model simulation
                            handle_oldTime,            & ! x%var(:)    -- time for the previous model time step
                            ! run time variables
                            err) bind(C, name="writeHRUToOutputStructure") 
  USE nrtype
  USE globalData,only:structInfo
  USE globalData,only:startWrite,endWrite
  USE globalData,only:maxLayers                               ! maximum number of layers
  USE globalData,only:maxSnowLayers                           ! maximum number of snow layers

  USE globalData,only:ixProgress                              ! define frequency to write progress
  USE globalData,only:ixRestart                               ! define frequency to write restart files
  USE globalData,only:gru_struc

  USE globalData,only:newOutputFile                           ! define option for new output files
  USE summa_alarms,only:summa_setWriteAlarms

  USE globalData,only:forc_meta,attr_meta,type_meta           ! metaData structures
  USE output_stats,only:calcStats                             ! module for compiling output statistics
  USE outputStrucWrite_module,only:writeData,writeBasin       ! module to write model output
  USE outputStrucWrite_module,only:writeTime                  ! module to write model time
  USE outputStrucWrite_module,only:writeRestart               ! module to write model Restart
  USE outputStrucWrite_module,only:writeParm                  ! module to write model parameters
  USE time_utils_module,only:elapsedSec                       ! calculate the elapsed time
  USE globalData,only:elapsedWrite                            ! elapsed time to write data
  USE output_structure_module,only:outputStructure
  USE netcdf_util_module,only:nc_file_close                   ! close netcdf file
  USE netcdf_util_module,only:nc_file_open                    ! open netcdf file
  USE var_lookup,only:maxvarFreq                              ! maximum number of output files

  implicit none
  integer(c_int),intent(in)             :: indxHRU               ! index of hru in GRU
  integer(c_int),intent(in)             :: indxGRU               ! index of the GRU
  integer(c_int),intent(in)             :: outputStep            ! index into the output Struc

  ! statistics variables
  type(c_ptr),intent(in),value          :: handle_forcStat       ! model forcing data
  type(c_ptr),intent(in),value          :: handle_progStat       ! model prognostic (state) variables
  type(c_ptr),intent(in),value          :: handle_diagStat       ! model diagnostic variables
  type(c_ptr),intent(in),value          :: handle_fluxStat       ! model fluxes
  type(c_ptr),intent(in),value          :: handle_indxStat       ! model indices
  type(c_ptr),intent(in),value          :: handle_bvarStat       ! basin-average variables
  ! primary data structures (scalars)
  type(c_ptr),intent(in),value          :: handle_timeStruct     ! x%var(:)     -- model time data
  type(c_ptr),intent(in),value          :: handle_forcStruct     ! x%var(:)     -- model forcing data
  ! primary data structures (variable length vectors)
  type(c_ptr),intent(in),value          :: handle_indxStruct     ! x%var(:)%dat -- model indices
  type(c_ptr),intent(in),value          :: handle_mparStruct     ! x%var(:)%dat -- model parameters
  type(c_ptr),intent(in),value          :: handle_progStruct     ! x%var(:)%dat -- model prognostic (state) variables
  type(c_ptr),intent(in),value          :: handle_diagStruct     ! x%var(:)%dat -- model diagnostic variables
  type(c_ptr),intent(in),value          :: handle_fluxStruct     ! x%var(:)%dat -- model fluxes
  ! basin-average structures
  type(c_ptr),intent(in),value          :: handle_bparStruct     ! x%var(:)     -- basin-average parameters
  type(c_ptr),intent(in),value          :: handle_bvarStruct     ! x%var(:)%dat -- basin-average variables
  ! local HRU data
  type(c_ptr),intent(in),value          :: handle_statCounter    ! x%var(:)
  type(c_ptr),intent(in),value          :: handle_outputTimeStep ! x%var(:)
  type(c_ptr),intent(in),value          :: handle_resetStats     ! x%var(:)
  type(c_ptr),intent(in),value          :: handle_finalizeStats  ! x%var(:)
  type(c_ptr),intent(in),value          :: handle_finshTime      ! x%var(:)    -- end time for the model simulation
  type(c_ptr),intent(in),value          :: handle_oldTime        ! x%var(:)    -- time for the previous model time step
  integer(c_int),intent(out)            :: err

  ! local pointers
  ! statistics variables 
  type(var_dlength), pointer            :: forcStat        ! model forcing data
  type(var_dlength), pointer            :: progStat        ! model prognostic (state) variables
  type(var_dlength), pointer            :: diagStat        ! model diagnostic variables
  type(var_dlength), pointer            :: fluxStat        ! model fluxes
  type(var_dlength), pointer            :: indxStat        ! model indices
  type(var_dlength), pointer            :: bvarStat        ! basin-average variabl
  ! primary data structures (scalars)
  type(var_i),pointer                   :: timeStruct      ! model time data
  type(var_d),pointer                   :: forcStruct      ! model forcing data
  ! primary data structures (variable length vectors)
  type(var_ilength),pointer             :: indxStruct      ! model indices
  type(var_dlength),pointer             :: mparStruct      ! model parameters
  type(var_dlength),pointer             :: progStruct      ! model prognostic (state) variables
  type(var_dlength),pointer             :: diagStruct      ! model diagnostic variables
  type(var_dlength),pointer             :: fluxStruct      ! model fluxes
  ! basin-average structures
  type(var_d),pointer                   :: bparStruct      ! basin-average parameters
  type(var_dlength),pointer             :: bvarStruct      ! basin-average variables
  ! local HRU data
  type(var_i),pointer                   :: statCounter     ! time counter for stats
  type(var_i),pointer                   :: outputTimeStep  ! timestep in output files
  type(flagVec),pointer                 :: resetStats      ! flags to reset statistics
  type(flagVec),pointer                 :: finalizeStats   ! flags to finalize statistics
  type(var_i),pointer                   :: finshTime       ! end time for the model simulation
  type(var_i),pointer                   :: oldTime         !

  ! local variables
  character(len=256)                    :: cmessage
  character(len=256)                    :: message 
  logical(lgt)                          :: defNewOutputFile=.false.
  logical(lgt)                          :: printRestart=.false.
  logical(lgt)                          :: printProgress=.false.
  character(len=256)                    :: restartFile       ! restart file name
  character(len=256)                    :: timeString        ! portion of restart file name that contains the write-out time
  integer(i4b)                          :: iStruct           ! index of model structure
  integer(i4b)                          :: iFreq             ! index of the output frequency
  ! convert the C pointers to Fortran pointers
  call c_f_pointer(handle_forcStat, forcStat)
  call c_f_pointer(handle_progStat, progStat)
  call c_f_pointer(handle_diagStat, diagStat)
  call c_f_pointer(handle_fluxStat, fluxStat)
  call c_f_pointer(handle_indxStat, indxStat)
  call c_f_pointer(handle_bvarStat, bvarStat)
  call c_f_pointer(handle_timeStruct, timeStruct)
  call c_f_pointer(handle_forcStruct, forcStruct)
  call c_f_pointer(handle_indxStruct, indxStruct)
  call c_f_pointer(handle_mparStruct, mparStruct)
  call c_f_pointer(handle_progStruct, progStruct)
  call c_f_pointer(handle_diagStruct, diagStruct)
  call c_f_pointer(handle_fluxStruct, fluxStruct)
  call c_f_pointer(handle_bparStruct, bparStruct)
  call c_f_pointer(handle_bvarStruct, bvarStruct)
  call c_f_pointer(handle_statCounter, statCounter)
  call c_f_pointer(handle_outputTimeStep, outputTimeStep)
  call c_f_pointer(handle_resetStats, resetStats)
  call c_f_pointer(handle_finalizeStats, finalizeStats)
  call c_f_pointer(handle_finshTime, finshTime)
  call c_f_pointer(handle_oldTime, oldTime)

  err=0; message='summa_manageOutputFiles/'
  ! identify the start of the writing

  ! Many variables get there values from summa4chm_util.f90:getCommandArguments()
  call summa_setWriteAlarms(oldTime%var, timeStruct%var, finshTime%var,  &   ! time vectors
                            newOutputFile,  defNewOutputFile,            &
                            ixRestart,      printRestart,                &   ! flag to print the restart file
                            ixProgress,     printProgress,               &   ! flag to print simulation progress
                            resetStats%dat, finalizeStats%dat,           &   ! flags to reset and finalize stats
                            statCounter%var,                             &   ! statistics counter
                            err, cmessage)                                  ! error control
  if(err/=0)then; message=trim(message)//trim(cmessage); return; endif

 ! If we do not do this looping we segfault - I am not sure why
  outputStructure(1)%finalizeStats(1)%gru(indxGRU)%hru(indxHRU)%tim(outputStep)%dat(:) = finalizeStats%dat(:)
 ! ****************************************************************************
 ! *** calculate output statistics
 ! ****************************************************************************
  do iStruct=1,size(structInfo)
    select case(trim(structInfo(iStruct)%structName))
      case('forc'); call calcStats(forcStat%var, forcStruct%var, statForc_meta, resetStats%dat, finalizeStats%dat, statCounter%var, err, cmessage)
      case('prog'); call calcStats(progStat%var, progStruct%var, statProg_meta, resetStats%dat, finalizeStats%dat, statCounter%var, err, cmessage)
      case('diag'); call calcStats(diagStat%var, diagStruct%var, statDiag_meta, resetStats%dat, finalizeStats%dat, statCounter%var, err, cmessage)
      case('flux'); call calcStats(fluxStat%var, fluxStruct%var, statFlux_meta, resetStats%dat, finalizeStats%dat, statCounter%var, err, cmessage)
      case('indx'); call calcStats(indxStat%var, indxStruct%var, statIndx_meta, resetStats%dat, finalizeStats%dat, statCounter%var, err, cmessage)     
    end select
    if(err/=0)then; message=trim(message)//trim(cmessage)//'['//trim(structInfo(iStruct)%structName)//']'; return; endif
  end do  ! (looping through structures)
    
  ! calc basin stats
  call calcStats(bvarStat%var(:), bvarStruct%var(:), statBvar_meta, resetStats%dat, finalizeStats%dat, statCounter%var, err, cmessage)
  if(err/=0)then; message=trim(message)//trim(cmessage)//'[bvar stats]'; return; endif
  
  ! write basin-average variables
  call writeBasin(indxGRU,indxHRU,outputStep,finalizeStats%dat, &
                  outputTimeStep%var,bvar_meta,bvarStat%var,bvarStruct%var,bvarChild_map,err,cmessage)
  if(err/=0)then; message=trim(message)//trim(cmessage)//'[bvar]'; return; endif

  ! ****************************************************************************
  ! *** write data
  ! ****************************************************************************
  call writeTime(indxGRU,indxHRU,outputStep,finalizeStats%dat, &
                time_meta,timeStruct%var,err,message)

  ! write the model output to the OutputStructure
  ! Passes the full metadata structure rather than the stats metadata structure because
  ! we have the option to write out data of types other than statistics.
  ! Thus, we must also pass the stats parent->child maps from childStruct.
  do iStruct=1,size(structInfo)
    select case(trim(structInfo(iStruct)%structName))
      case('forc'); call writeData(indxGRU,indxHRU,outputStep,"forc",finalizeStats%dat,&
                    maxLayers,forc_meta,forcStat,forcStruct,forcChild_map,indxStruct,err,cmessage)
      case('prog'); call writeData(indxGRU,indxHRU,outputStep,"prog",finalizeStats%dat,&
                    maxLayers,prog_meta,progStat,progStruct,progChild_map,indxStruct,err,cmessage)
      case('diag'); call writeData(indxGRU,indxHRU,outputStep,"diag",finalizeStats%dat,&
                    maxLayers,diag_meta,diagStat,diagStruct,diagChild_map,indxStruct,err,cmessage)
      case('flux'); call writeData(indxGRU,indxHRU,outputStep,"flux",finalizeStats%dat,&
                    maxLayers,flux_meta,fluxStat,fluxStruct,fluxChild_map,indxStruct,err,cmessage)
      case('indx'); call writeData(indxGRU,indxHRU,outputStep,"indx",finalizeStats%dat,&
                    maxLayers,indx_meta,indxStat,indxStruct,indxChild_map,indxStruct,err,cmessage)
    end select
    if(err/=0)then 
      message=trim(message)//trim(cmessage)//'['//trim(structInfo(iStruct)%structName)//']'
      return
    endif
  end do  ! (looping through structures)

  ! *****************************************************************************
  ! *** update counters
  ! *****************************************************************************

  ! increment output file timestep
  do iFreq = 1,maxvarFreq
    statCounter%var(iFreq) = statCounter%var(iFreq)+1
    if(finalizeStats%dat(iFreq)) outputTimeStep%var(iFreq) = outputTimeStep%var(iFreq) + 1
  end do

  ! if finalized stats, then reset stats on the next time step
  resetStats%dat(:) = finalizeStats%dat(:)

  ! save time vector
  oldTime%var(:) = timeStruct%var(:)

end subroutine writeHRUToOutputStructure

end module HRUwriteoOutput_module