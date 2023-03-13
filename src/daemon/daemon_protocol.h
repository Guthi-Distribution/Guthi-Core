#pragma once 

enum EMessage
{
    GetFile              = 0, 
    CheckIfInCache       = 1,
    RequestFileMetadata  = 2,
    NoSuchResourceExists = 3,
    TrackedFileChanged   = 4, // Response : followed by (modification time) File Name 
    TrackThisFile        = 5, // Request  : same as above			     
    EchoMessage          = 6, 
    Continuation         = 7,  // Response : Continuation of previous message 
    InvalidRequest       = 8, 
    GetCachedFile        = 9, 
    ResetConnection      = 10
};
