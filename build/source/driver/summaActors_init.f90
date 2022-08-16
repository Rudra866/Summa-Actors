module summaActors_init
! used to declare and allocate summa data structures and initialize model state to known values
USE,intrinsic :: iso_c_binding
USE nrtype          ! variable types, etc.
USE data_types,only:&
                    ! no spatial dimension
                    var_i,               & ! x%var(:)            (i4b)
                    var_i8,              & ! x%var(:)            (i8b)
                    var_d,               & ! x%var(:)            (dp)
                    var_ilength,         & ! x%var(:)%dat        (i4b)
                    var_dlength            ! x%var(:)%dat        (dp)
                    
! access missing values
USE globalData,only:integerMissing   ! missing integer
USE globalData,only:realMissing      ! missing double precision number
! named variables for run time options
USE globalData,only:iRunModeFull,iRunModeGRU,iRunModeHRU
! metadata structures
USE globalData,only:time_meta,forc_meta,attr_meta,type_meta ! metadata structures
USE globalData,only:prog_meta,diag_meta,flux_meta,id_meta   ! metadata structures
USE globalData,only:mpar_meta,indx_meta                     ! metadata structures
USE globalData,only:bpar_meta,bvar_meta                     ! metadata structures
USE globalData,only:averageFlux_meta                        ! metadata for time-step average fluxes
! statistics metadata structures
USE globalData,only:statForc_meta                           ! child metadata for stats
USE globalData,only:statProg_meta                           ! child metadata for stats
USE globalData,only:statDiag_meta                           ! child metadata for stats
USE globalData,only:statFlux_meta                           ! child metadata for stats
USE globalData,only:statIndx_meta                           ! child metadata for stats
USE globalData,only:statBvar_meta                           ! child metadata for stats
! maxvarFreq 
USE var_lookup,only:maxVarFreq                               ! # of available output frequencies

! safety: set private unless specified otherwise
implicit none
private
public::summaActors_initialize
contains

 ! used to declare and allocate summa data structures and initialize model state to known values
 subroutine summaActors_initialize(&
                        indxGRU,            & !  Index of HRU's GRU parent
                        num_steps,          &
  						          ! statistics structures
                        handle_forcStat,    & !  model forcing data
                        handle_progStat,    & !  model prognostic (state) variables
                        handle_diagStat,    & !  model diagnostic variables
                        handle_fluxStat,    & !  model fluxes
                        handle_indxStat,    & !  model indices
                        handle_bvarStat,    & !  basin-average variables
                        ! primary data structures (scalars)
                        handle_timeStruct,  & !  model time data
                        handle_forcStruct,  & !  model forcing data
                        handle_attrStruct,  & !  local attributes for each HRU
                        handle_typeStruct,  & !  local classification of soil veg etc. for each HRU
                        handle_idStruct,    & ! 
                        ! primary data structures (variable length vectors)
                        handle_indxStruct,  & !  model indices
                        handle_mparStruct,  & !  model parameters
                        handle_progStruct,  & !  model prognostic (state) variables
                        handle_diagStruct,  & !  model diagnostic variables
                        handle_fluxStruct,  & !  model fluxes
                        ! basin-average structures
                        handle_bparStruct,  & !  basin-average parameters
                        handle_bvarStruct,  & !  basin-average variables
                        ! ancillary data structures
                        handle_dparStruct,  & !  default model parameters
                        ! local HRU data structures
                        handle_startTime,   & ! start time for the model simulation
                        handle_finshTime,   & ! end time for the model simulation
                        handle_refTime,     & ! reference time for the model simulation
                        handle_oldTime,     & ! time for the previous model time step
                        ! miscellaneous variables
                        err) bind(C,name='summaActors_initialize')
  ! ---------------------------------------------------------------------------------------
  ! * desired modules
  ! ---------------------------------------------------------------------------------------
  ! data types
  USE nrtype                                                  ! variable types, etc.
  USE time_utils_module,only:elapsedSec                       ! calculate the elapsed time
  ! subroutines and functions: allocate space
  USE allocspace4chm_module,only:allocGlobal              ! module to allocate space for global data structures
  USE allocspace4chm_module,only:allocLocal
  ! timing variables
  USE globalData,only:startInit,endInit                       ! date/time for the start and end of the initialization
  USE globalData,only:elapsedRead                             ! elapsed time for the data read
  USE globalData,only:elapsedWrite                            ! elapsed time for the stats/write
  USE globalData,only:elapsedPhysics                          ! elapsed time for the physics
  ! miscellaneous global data
  USE globalData,only:gru_struc                               ! gru-hru mapping structures
  USE globalData,only:structInfo                              ! information on the data structures
  USE globalData,only:numtim
  USE var_lookup,only:maxvarFreq                              ! maximum number of output files
  USE globalData,only:startTime,finshTime,refTime,oldTime
  
  implicit none
  
  ! ---------------------------------------------------------------------------------------
  ! * variables from C++
  ! ---------------------------------------------------------------------------------------
  integer(c_int),intent(in)                  :: indxGRU                    ! indx of the parent GRU
  integer(c_int),intent(out)                 :: num_steps                  ! number of steps in model, local to the HRU                 
    ! statistics structures
  type(c_ptr), intent(in), value             :: handle_forcStat !  model forcing data
  type(c_ptr), intent(in), value             :: handle_progStat !  model prognostic (state) variables
  type(c_ptr), intent(in), value             :: handle_diagStat !  model diagnostic variables
  type(c_ptr), intent(in), value             :: handle_fluxStat !  model fluxes
  type(c_ptr), intent(in), value             :: handle_indxStat !  model indices
  type(c_ptr), intent(in), value             :: handle_bvarStat !  basin-average variables
  ! primary data structures (scalars)
  type(c_ptr), intent(in), value             :: handle_timeStruct !  model time data
  type(c_ptr), intent(in), value             :: handle_forcStruct !  model forcing data
  type(c_ptr), intent(in), value             :: handle_attrStruct !  local attributes for each HRU
  type(c_ptr), intent(in), value             :: handle_typeStruct !  local classification of soil veg etc. for each HRU
  type(c_ptr), intent(in), value             :: handle_idStruct ! 
  ! primary data structures (variable length vectors)
  type(c_ptr), intent(in), value             :: handle_indxStruct !  model indices
  type(c_ptr), intent(in), value             :: handle_mparStruct !  model parameters
  type(c_ptr), intent(in), value             :: handle_progStruct !  model prognostic (state) variables
  type(c_ptr), intent(in), value             :: handle_diagStruct !  model diagnostic variables
  type(c_ptr), intent(in), value             :: handle_fluxStruct !  model fluxes
  ! basin-average structures
  type(c_ptr), intent(in), value             :: handle_bparStruct !  basin-average parameters
  type(c_ptr), intent(in), value             :: handle_bvarStruct !  basin-average variables
  ! ancillary data structures
  type(c_ptr), intent(in), value             :: handle_dparStruct !  default model parameters
  ! local hru data structures
  type(c_ptr), intent(in), value             :: handle_startTime  ! start time for the model simulation
  type(c_ptr), intent(in), value             :: handle_finshTime ! end time for the model simulation
  type(c_ptr), intent(in), value             :: handle_refTime    ! reference time for the model simulation
  type(c_ptr), intent(in), value             :: handle_oldTime    ! time for the previous model time step 
  integer(c_int),intent(inout)               :: err  
  ! ---------------------------------------------------------------------------------------
  ! * Fortran Variables For Conversion
  ! ---------------------------------------------------------------------------------------
  type(var_dlength),pointer                  :: forcStat                   !  model forcing data
  type(var_dlength),pointer                  :: progStat                   !  model prognostic (state) variables
  type(var_dlength),pointer                  :: diagStat                   !  model diagnostic variables
  type(var_dlength),pointer                  :: fluxStat                   !  model fluxes
  type(var_dlength),pointer                  :: indxStat                   !  model indices
  type(var_dlength),pointer                  :: bvarStat                   !  basin-average variabl
  ! primary data structures (scalars)
  type(var_i),pointer                        :: timeStruct                 !  model time data
  type(var_d),pointer                        :: forcStruct                 !  model forcing data
  type(var_d),pointer                        :: attrStruct                 !  local attributes for each HRU
  type(var_i),pointer                        :: typeStruct                 !  local classification of soil veg etc. for each HRU
  type(var_i8),pointer                       :: idStruct                   ! 
  ! primary data structures (variable length vectors)
  type(var_ilength),pointer                  :: indxStruct                 !  model indices
  type(var_dlength),pointer                  :: mparStruct                 !  model parameters
  type(var_dlength),pointer                  :: progStruct                 !  model prognostic (state) variables
  type(var_dlength),pointer                  :: diagStruct                 !  model diagnostic variables
  type(var_dlength),pointer                  :: fluxStruct                 !  model fluxes
  ! basin-average structures
  type(var_d),pointer                        :: bparStruct                 !  basin-average parameters
  type(var_dlength),pointer                  :: bvarStruct                 !  basin-average variables
  ! ancillary data structures
  type(var_d),pointer                        :: dparStruct                 !  default model parameters
  ! local HRU data structures
  type(var_i),pointer                        :: startTime_hru              ! start time for the model simulation
  type(var_i),pointer                        :: finishTime_hru             ! end time for the model simulation
  type(var_i),pointer                        :: refTime_hru                ! reference time for the model simulation
  type(var_i),pointer                        :: oldTime_hru                ! time from previous step
  ! ---------------------------------------------------------------------------------------
  ! * Local Subroutine Variables
  ! ---------------------------------------------------------------------------------------
  character(LEN=256)                       :: message                    ! error message
  character(LEN=256)                       :: cmessage                   ! error message of downwind routine
  integer(i4b)                             :: iStruct                    ! looping variables
  ! ---------------------------------------------------------------------------------------
  ! * Convert From C++ to Fortran
  ! ---------------------------------------------------------------------------------------
  call c_f_pointer(handle_forcStat,   forcStat)
  call c_f_pointer(handle_progStat,   progStat)
  call c_f_pointer(handle_diagStat,   diagStat)
  call c_f_pointer(handle_fluxStat,   fluxStat)
  call c_f_pointer(handle_indxStat,   indxStat)
  call c_f_pointer(handle_bvarStat,   bvarStat)
  call c_f_pointer(handle_timeStruct, timeStruct)
  call c_f_pointer(handle_forcStruct, forcStruct)
  call c_f_pointer(handle_attrStruct, attrStruct)
  call c_f_pointer(handle_typeStruct, typeStruct)
  call c_f_pointer(handle_idStruct,   idStruct)
  call c_f_pointer(handle_indxStruct, indxStruct)
  call c_f_pointer(handle_mparStruct, mparStruct)
  call c_f_pointer(handle_progStruct, progStruct)
  call c_f_pointer(handle_diagStruct, diagStruct)
  call c_f_pointer(handle_fluxStruct, fluxStruct)
  call c_f_pointer(handle_bparStruct, bparStruct)
  call c_f_pointer(handle_bvarStruct, bvarStruct)
  call c_f_pointer(handle_dparStruct, dparStruct)
  call c_f_pointer(handle_startTime,  startTime_hru)
  call c_f_pointer(handle_finshTime,  finishTime_hru)
  call c_f_pointer(handle_refTime,    refTime_hru)
  call c_f_pointer(handle_oldTime,    oldTime_hru)

  ! ---------------------------------------------------------------------------------------
  ! initialize error control
  err=0; message='summaActors_initialize/'

  ! initialize the start of the initialization
  call date_and_time(values=startInit)

  ! initialize the elapsed time for cumulative quantities
  elapsedRead=0._dp
  elapsedWrite=0._dp
  elapsedPhysics=0._dp

  ! copy the number of the steps for the hru
  num_steps = numtim

  ! *****************************************************************************
  ! *** allocate space for data structures
  ! *****************************************************************************

  ! allocate time structures
  do iStruct=1,4
  select case(iStruct)
    case(1); call allocLocal(time_meta, startTime_hru, err=err, message=cmessage)  ! start time for the model simulation
    case(2); call allocLocal(time_meta, finishTime_hru, err=err, message=cmessage)  ! end time for the model simulation
    case(3); call allocLocal(time_meta, refTime_hru,   err=err, message=cmessage)  ! reference time for the model simulation
    case(4); call allocLocal(time_meta, oldTime_hru,   err=err, message=cmessage)  ! time from the previous step
  end select
  if(err/=0)then; message=trim(message)//trim(cmessage); return; endif
  end do  ! looping through time structures

  ! copy the time variables set up by the job_actor
  startTime_hru%var(:) = startTime%var(:)
  finishTime_hru%var(:) = finshTime%var(:)
  refTime_hru%var(:) = refTime%var(:)
  oldTime_hru%var(:) = oldTime%var(:)


  ! get the number of snow and soil layers
  associate(&
  nSnow => gru_struc(indxGRU)%hruInfo(1)%nSnow, & ! number of snow layers for each HRU
  nSoil => gru_struc(indxGRU)%hruInfo(1)%nSoil  ) ! number of soil layers for each HRU

  ! allocate other data structures
  do iStruct=1,size(structInfo)
  ! allocate space  
  select case(trim(structInfo(iStruct)%structName))    
    case('time'); call allocLocal(time_meta,timeStruct,err=err,message=cmessage)     ! model forcing data
    case('forc'); call allocLocal(forc_meta,forcStruct,nSnow,nSoil,err,cmessage);    ! model forcing data
    case('attr'); call allocLocal(attr_meta,attrStruct,nSnow,nSoil,err,cmessage);    ! local attributes for each HRU
    case('type'); call allocLocal(type_meta,typeStruct,nSnow,nSoil,err,cmessage);    ! classification of soil veg etc.
    case('id'  ); call allocLocal(id_meta,idStruct,nSnow,nSoil,err,cmessage);        ! local values of hru and gru IDs
    case('mpar'); call allocLocal(mpar_meta,mparStruct,nSnow,nSoil,err,cmessage);    ! model parameters
    case('indx'); call allocLocal(indx_meta,indxStruct,nSnow,nSoil,err,cmessage);    ! model variables
    case('prog'); call allocLocal(prog_meta,progStruct,nSnow,nSoil,err,cmessage);    ! model prognostic (state) variables
    case('diag'); call allocLocal(diag_meta,diagStruct,nSnow,nSoil,err,cmessage);    ! model diagnostic variables
    case('flux'); call allocLocal(flux_meta,fluxStruct,nSnow,nSoil,err,cmessage);    ! model fluxes
    case('bpar'); call allocLocal(bpar_meta,bparStruct,nSnow=0,nSoil=0,err=err,message=cmessage);  ! basin-average params 
    case('bvar'); call allocLocal(bvar_meta,bvarStruct,nSnow=0,nSoil=0,err=err,message=cmessage);  ! basin-average variables
    case('deriv'); cycle
    case default; err=20; message='unable to find structure name: '//trim(structInfo(iStruct)%structName)
  end select
  ! check errors
  if(err/=0)then
    message=trim(message)//trim(cmessage)//'[structure =  '//trim(structInfo(iStruct)%structName)//']'
    return
  endif
  end do  ! looping through data structures

  ! allocate space for default model parameters
  ! NOTE: This is done here, rather than in the loop above, because dpar is not one of the "standard" data structures
  call allocLocal(mpar_meta,dparStruct,nSnow,nSoil,err,cmessage);    ! default model parameters
    if(err/=0)then
    message=trim(message)//trim(cmessage)//' [problem allocating dparStruct]'
    return
  endif

  ! *****************************************************************************
  ! *** allocate space for output statistics data structures
  ! *****************************************************************************
  ! loop through data structures
  do iStruct=1,size(structInfo)
    ! allocate space
    select case(trim(structInfo(iStruct)%structName))
      case('forc'); call allocLocal(statForc_meta(:)%var_info,forcStat,nSnow,nSoil,err,cmessage);    ! model forcing data
      case('prog'); call allocLocal(statProg_meta(:)%var_info,progStat,nSnow,nSoil,err,cmessage);    ! model prognostic 
      case('diag'); call allocLocal(statDiag_meta(:)%var_info,diagStat,nSnow,nSoil,err,cmessage);    ! model diagnostic
      case('flux'); call allocLocal(statFlux_meta(:)%var_info,fluxStat,nSnow,nSoil,err,cmessage);    ! model fluxes
      case('indx'); call allocLocal(statIndx_meta(:)%var_info,indxStat,nSnow,nSoil,err,cmessage);    ! index vars
      case('bvar'); call allocLocal(statBvar_meta(:)%var_info,bvarStat,nSnow=0,nSoil=0,err=err,message=cmessage);  ! basin-average variables
      case default; cycle
    end select
    ! check errors
    if(err/=0)then
      message=trim(message)//trim(cmessage)//'[statistics for =  '//trim(structInfo(iStruct)%structName)//']'
      return
    endif
  end do ! iStruct

  ! identify the end of the initialization
  call date_and_time(values=endInit)

  ! end association to info in data structures
  end associate

 end subroutine summaActors_initialize

end module summaActors_init
