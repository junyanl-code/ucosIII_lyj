// SNMP应用层程序

#include <includes.h>
#include "arch/cc.h"
#include "Snmp\snmp_SNMP.h"
#include "Snmp\snmp_mib.h"
#define ERR_VGO(s, v, t) goto t
/*define Get,Set/GetNext GetBulk Request*******/
#define NoGetOrSetRequest      0
#define GetSetOrRequest        1

#define NoGetBulkOrNextRequest 0
#define GetBulkOrNextRequest   1

#define GetBulkRequestMark     1

/* Local copies of Agent Context information */
static u16_t    nmibs;
//static uint16     trapv, trapid;
static const    MIB** mibs;

extern u8_t*    Snmp_Context;

extern AGENT_CONTEXT snmp_ac;

///* Private fuction,so much range in this file */
//static void Judge_Bulk_Parameters(u32_t sMaxRepeat, u32_t sNonRepeat, u32_t sRemain, s8_t sVb_count);


/***********Fuction :ussSNMPAgentInit*************/
/***********Descript:SNMPAgent Init  *************/

void ussSNMPAgentInit( void )
{
    /* Save global context information for Agent */
    mibs = snmp_ac.mibs;
    nmibs = snmp_ac.nummibs;
    //trapv = snmp_ac.trapv;
    //trapid = 1;
}

u8_t test1[16] = {0};
u8_t test2[16] = {0};
s16_t isAccessAllowed( const u8_t* contextName )
{
    u8_t i = 0;
    for( i = 0; i < 16; i++ )
    {
        test1[i] = Snmp_Context[i];
        test2[i] = contextName[i];
    }
    if( sncompare( contextName, ( u8_t* )Snmp_Context, 16 ) == 0 )
    {
        return accessAllowed;
    }
    return noSuchContext;
}


/**************Fuction: static s8_t BulkRep_Break**************************/
/**************Description: if return 1,so breakk,or not continue loop         */
/**************Parameters : sbulkflag sMaxRepeat, sNonRepeat, sRemain, sVbcounter,sReapcounter*************/
/**************Return : NULL                                          *************/
//static s8_t BulkRep_Break(u8_t sBulkflag, u32_t sMaxRep, u32_t sNonRep, s8_t sVbCount, s8_t sRepCount)
//{
//     if(sBulkflag != GetBulkRequestMark)
//        return 1;                     /* jump in break statments*/
//     
//     if(sVbCount+1 <= sNonRep)
//        return 1;
//     
//     if((sRepCount % sMaxRep == 0)&& (sRepCount != 0))
//     {
//        sRepCount++;
//        return 1;
//     }
//     else
//     {
//        sRepCount++;
//        return 0;
//     }
//     
//     //(sRepCount % sMaxRep == 0)&& (sRepCount != 0)? (sRepCount++ ,return 1 ):return 0;/* */  
//}
 

/**************Fuction: static void Judge_Bulk_Parameters**************************/
/**************Description: It judge that the parameters of bulk is or not correct*/
/**************Parameters : sMaxRepeat, sNonRepeat, sRemain, sVb_count*************/
/**************Return : NULL                                          *************/
//static void Judge_Bulk_Parameters(u32_t sMaxRepeat, u32_t sNonRepeat, u32_t sRemain, s8_t sVb_count )
//{
//            //if(sVb_count == 0)
//            //   return;
//            if(sMaxRepeat < 1)
//	    {
//			sNonRepeat = sVb_count;
//			sMaxRepeat = 1;
//	    }
//	    else
//	    {
//			//if(sNonRepeat < 0)
//			//{
//			//	sNonRepeat = 0;
//			//}
//			//else 
//                        if(sNonRepeat >= sVb_count)
//			{
//				sNonRepeat = sVb_count;
//				sMaxRepeat = 1;
//			}
//			else
//			{
//				sRemain = sVb_count - sNonRepeat;
//				if((sNonRepeat + sRemain*sMaxRepeat) >= MAXOID)
//				{
//					sMaxRepeat = (MAXOID - sNonRepeat) / sRemain;
//				}
//			}
//	     }
//}
             
/*********Fuction:  inReqOutRep**************************************/
/*********Paramet:  obp that is the point of  end of the outbuf  *****/
/*********Description: this is that decode or encode snmp data  *****/

s16_t inReqOutRep( u8_t** obp, u16_t olen, const u8_t* ibp, u16_t ilen )
{
  
    const   MIB*     mibp = NULL;
    const   MIBVAR*  mvp;
    const   MIBTAB*  mtp;
    const   u8_t*    varbinds[MAXVAR];
    const   u8_t*    ibpend, *chkp, *chkp2, *inp;
    u8_t*   obpend, *outp, *valp, *cp, *soutp;
    u8_t*   contextName;
    u16_t*  emptyflag;
    u32_t   ul1, reqid, nonrep, maxrep;
    //s16_t   nonrep, maxrep,remain;
    s32_t   sl1, getsetflag;
    s16_t   i1, i2, i3, len, len2, mibix, tabix, ixlen;
    
    //it mean the type of pdu,the first "type" as follow.
    u8_t    type, version, errix, errcode, nflag;//bulkflag;
    s8_t    vindex;
    s8_t    vbcounter;//repcounter;
    u16_t  i;
    s32_t  r;
    
    
    i3 = 0;
    errix = errcode = nflag = 0;//bulkflag = 0;
    
    //definite the end of input buffer  
    ibpend = ibp + ilen;                /* Read from start to end */
    
    //definite the start of input buffer 
    inp = ibp;
    
    //definite the end of output buffer
    outp = *obp;
    
    //definite the start of output buffer
    obpend = *obp - olen;
    
    contextName = obpend + 1;
    *contextName = 0;
    
    /* it is the byte that judge the first snmp data is or not Sqquence(0x30) */
    len = snmpReadLength( &inp, Sequence );
    
    /**here is the byte that indicate the total len of data */
    if( len <= 0 )
    {
err_parse:
        return 0;
    }
    
    chkp = inp + len;                             /* Check length of message */
    
    /**where is the postion in buffer that indicate the snmp version */
    len = snmpReadInt( &ul1, sizeof( ul1 ), &inp, Integer );
    
    
    if( len <= 0 || len > ( u8_t )sizeof( ul1 ) )/*error */
    {
        ERR_VGO( "msgVersion Length", len, err_parse );
    }
    version = ul1;
    
    /**it judge that is 0 or 1,the byte of version data.*/
    if( version != 0 && version != 1 )
    {
        ERR_VGO( "Bad version", version, err_parse );
    }
    
    /**that is get from the data,compare to local string */
    len = snmpReadVal( contextName, olen, &inp, String );
    if( len < 0 || ( u16_t )len > olen )
    {
        ERR_VGO( "ContextName Length", len, err_parse );
    }
    
    /**that is null,the end of context name.            */
    contextName[len] = 0;           /* Null terminate contextName */
    
    /* PDU Type */
    type = *inp;
    len = snmpReadLength( &inp, type );
    if( len <= 0 )
    {
        ERR_VGO( "Type Field Length", len, err_parse );
    }
    chkp2 = inp + len;
    /* Packet Id */
    len = snmpReadInt( &reqid, sizeof( reqid ), &inp, Integer );
    if( len < 1 )
    {
        ERR_VGO( "Request-ID Length", len, err_parse );
    }
    /* Error value */
    len = snmpReadInt( &nonrep, sizeof( nonrep ), &inp, Integer );
    if( len <= 0 )
    {
        ERR_VGO( "Error value Length", len, err_parse );
    }
    /* Error index */
    len = snmpReadInt( &maxrep, sizeof( maxrep ), &inp, Integer );
    if( len <= 0 )
    {
        ERR_VGO( "Error Index Length", len, err_parse );
    }
    /* Data */
    len = snmpReadLength( &inp, Sequence );
    if( len < 0 )
    {
        ERR_VGO( "PDU Data Length", len, err_parse );
    }
    /* Variable bindings */
    vindex      = 0;
    vbcounter   = 0;
    //repcounter  = 0;
    varbinds[0] = inp;
    while( inp < ibpend )
    {
        /* SEQUENCE OF { OID, Value } */
        varbinds[vindex++] = inp;   /* Save bindings */
        
        /* SEQUENCE */
        len2 = snmpReadLength( &inp, Sequence );
        if( len2 <= 0 )
        {
              ERR_VGO( "Variable", vindex, err_parse );
        }
        
        inp += len2;
        if( vindex >= MAXVAR )
        {
              errcode = tooBig;
              errix = 0;
              vindex = 0;            /* Skip variable processing */
              break;
        }
    }
    
    if( inp > ibpend )              /*here is out range of buffer that must jump in error statments*/
    {
        goto err_parse;
    }
    if( inp != chkp )              
    {
        goto err_parse;
    }
    nflag = 0;
    
    /****There is defined the flag,so much as Get/Set Request or GetBulk/GetNext.*****/
    switch( type )
    {
        case SetRequest:
        case GetRequest:
            getsetflag = GetSetOrRequest;
        break;
            
        case GetNextRequest:
            getsetflag = NoGetOrSetRequest;            /*in case,the variable "getsetflag" should set */
            nflag = GetBulkOrNextRequest;
        break;
           
            
        case GetBulkRequest:
        {
            if(version != 1)
            {
                  errix  = 0;
                  vindex = 0;                          /* Skip variable processing */
                  errcode = DECODE_ASN_PARSE_ERROR;
            }
            else
            {
                  /****It judge that the parameters of repeat is or not correct*****/
                  //Judge_Bulk_Parameters(maxrep, nonrep, remain, vindex);
                  
                  /****this pdu type have been setted to GetNextRequest         ****/
                  getsetflag = NoGetOrSetRequest;
                  nflag      = GetBulkOrNextRequest;   /*the pdu type have should set GetNextRequest*/
                  //bulkflag   = GetBulkRequestMark;         
            }
        }
        break;
        
        default:
            goto err_parse;
            //break;
    };
    
    
    
    /* Process variables from last to first. */
    while(vbcounter < vindex )
    {
        //inp = varbinds[vindex - 1];               /* Point to variable binding */
        inp = varbinds[vbcounter];
 
        snmpReadLength( &inp, Sequence );
        
        len2 = snmpReadLength( &inp, Identifier );
        
        if( len2 <= 0 )
        {
            ERR_VGO( "Variable OID Length", len2, err_parse );
        }
        
        mvp = 0;                                /*  so this Initial the address pointed to mibvar that is null*/
        mtp = 0;                                /*  so this Initial the address pointed to mibtab that is null*/
        tabix = -1;                             /*  Just there must have been setted the column of table that is -1*/                                                       
        soutp = outp;                           /*  soutp point to outp*/
        /* Search through ordered MIBs until variable is found */
        for( mibix = 0; mibix < ( s16_t )nmibs && mvp == 0; mibix++ )
        {
            /**it get the point of table  ********/
            mibp = mibs[mibix];
            
            /**it search from the start to end and find which is compare to the OID */
            for( i = 0; i < mibp->numvars(); i++ )
            {
                mvp = &mibp->mvp[i];
                
                /****it hop the next mib node that is NOACCESS type.****/
                if( mvp->opt == NOACCESS )
                {
                    continue;
                }
                
                r = snmpVCompare( inp, len2, mvp->oid.name, mvp->oid.nlen );
                
                /***** this is perfectly match****************/
                if( r == 0 )
                {
                    /* Perfectily match */
                    tabix = ( s16_t )mvp->oid.name[mvp->oid.nlen - 1] - 1;
                    
                    /* it tell us that is a scalar****/
                    if( tabix < 0 )
                    {
                        
                       /* scalar var */
                        if( getsetflag )
                        {
                            tabix = -1;
                            break;
                        }
                    }
                    else
                    {
                        /* it tell us that is a table*/
                        /* GetnextRequest       ******/ 
                        if( getsetflag == 0 )
                        {
                            sl1 = snmpFindOID( ( const u8_t** )&mtp,
                                               ( u8_t* )mibp->mtp, sizeof( MIBTAB ), mibp->numtabs(),
                                               ( u8_t* )mvp->oid.name, mvp->oid.nlen - 2 );
                            
                            /* If not table found or not an exact match */
                            if( mtp == 0 || sl1 )
                            {
                                /* Generate a general error */
                                i1 = genErr;
                                ERR_VGO( "No matching table for var!", sl1, err_val );
                            }
                            if( mtp->empty )
                            {
                                /* it set to 0,the address of the table mvp (mibvar)*/
                                emptyflag = ( u16_t* )( mtp->empty );
                                
                                /* it indicate no mib node that are search*/
                                if( !( *emptyflag ) )
                                {
                                    mvp = 0;
                                }
                                else
                                {
                                    tabix = 0;
                                    break;      
                                }
                            }
                            else
                            {
                                tabix = 0;
                                break;       //found
                            }
                        }
                    }
                }
                // 包含基准
                else if( ( r > 0 ) && ( r < 255 ) )
                {
                    tabix = ( s16_t )mvp->oid.name[mvp->oid.nlen - 1] - 1;
                    if( tabix >= 0 )
                    {
                        /* Find the table using found OID */
                        sl1 = snmpFindOID( ( const u8_t** )&mtp,
                                           ( u8_t* )mibp->mtp, sizeof( MIBTAB ), mibp->numtabs(),
                                           ( u8_t* )mvp->oid.name, mvp->oid.nlen - 2 );
                        /* If not table found or not an exact match */
                        if( mtp == 0 || sl1 )
                        {
                            /* Generate a general error */
                            //repcounter = 0;
                            i1 = genErr;
                            ERR_VGO( "No matching table for var!", sl1, err_val );
                        }
                        
                        ixlen = len2 - mvp->oid.nlen;
                        /* Repeat Counter increase   */
                       //((bulkflag == GetBulkRequestMark)&&(vbcounter+1 > nonrep))? repcounter++ : NULL;
                        /* Find Index number into MIB table */
                        sl1 = snmpFindIndex( &tabix, mtp, mibp, mvp,inp + mvp->oid.nlen, ( u8_t )ixlen, nflag );
                        if( sl1 )
                        {
                            break;    //found
                        }
                        else
                        {
                            mvp = 0;
                        }
                    }
                }
                // 被基准包含或比基准小
                else if( r < 0 )
                {
                    tabix = ( s16_t )mvp->oid.name[mvp->oid.nlen - 1] - 1;
                    if( tabix < 0 )
                    {
                        /* scalar var */
                        if( getsetflag )
                        {
                            mvp = 0;
                            break;
                        }
                        else
                        {
                         //((bulkflag == GetBulkRequestMark)&&(vbcounter+1 > nonrep))? repcounter++ : NULL;
                            tabix = -1;
                            break;
                          
                        }
                    }
                    else
                    {
                        /* table var */
                        if( getsetflag )
                        {
                            mvp = 0;
                            break;
                        }
                        else
                        {
                            sl1 = snmpFindOID( ( const u8_t** )&mtp,
                                               ( u8_t* )mibp->mtp, sizeof( MIBTAB ), mibp->numtabs(),
                                               ( u8_t* )mvp->oid.name, mvp->oid.nlen - 2 );
                            /* If not table found or not an exact match */
                            if( mtp == 0 || sl1 )
                            {
                                /* Generate a general error */
                                i1 = genErr;
                                ERR_VGO( "No matching table for var!", sl1, err_val );
                            }
                            if( mtp->empty )
                            {
                                emptyflag = ( u16_t* )( mtp->empty );
                                if( !( *emptyflag ) )
                                {
                                    mvp = 0;
                                }
                                else
                                {
                                    tabix = 0;
                                    break;      //found
                                }
                            }
                            else
                            {
                                tabix = 0;
                                break;         //found
                            }
                        }
                    }
                }
                mvp = 0;
            }
        }
        /* If mvp doesn't match in any MIB */
        if( mvp == 0 )
        {
            /* If SNMPv1 */
            //            if( version == 0 )
            if( version == 1 )
                /* Generate a noSuchName error */
            {
                i1 = noSuchName;
            }
            /* Else if SNMPv2 or SNMPv3 */
            else
                /* Generate a NULL variable binding value */
            {
                i1 = 0;
            }
            ERR_VGO( "End of MIB for var", vbcounter, err_val );
        }
        /* Performance view-based access string */
        i1 = isAccessAllowed( contextName );
        switch( i1 )
        {
            case accessAllowed:
                break;
            case noSuchContext:
            default:
                i1 = noAccess;
                ERR_VGO( "VACM failure for var", vbcounter, err_val );
        }
        
        
        valp = mvp->ptr;
        /* Check length fields */
        if( type == SetRequest )
        {
            inp += len2;    /* Point to data field */
            chkp2 = inp;
            if( mvp->type == OctetString )
            {
                len = snmpReadLength( ( const u8_t** )&chkp2, String );
            }
            else
            {
                len = snmpReadLength( ( const u8_t** )&chkp2, mvp->type );
            }
            if( len < 0 )
            {
                ERR_VGO( "Bad length field for var", vbcounter, err_parse );
            }
            i1 = 0;
            if( ( mvp->opt & W ) == 0 )
            {
                i1 = readOnly;
            }
            else if( mvp->type == Integer || mvp->type == Counter ||
                     mvp->type == Ticks || mvp->type == Gauge ||
                     mvp->type == OctetString )
            {
                if( len > mvp->len || len < 1 )
                {
                    i1 = wrongLength;
                }
                if( mvp->type == Integer && *chkp2 & 0x80 )
                {
                    i3 = 0xff;    /* Value is negative */
                }
                else
                {
                    i3 = 0;    /* Value is positive */
                }
            }
            else if( mvp->type == String )
            {
                if( len >= mvp->len )
                {
                    i1 = wrongLength;
                }
            }
            else if( len != mvp->len )
            {
                i1 = wrongLength;
            }
            if( i1 )
            {
                ERR_VGO( "Incorrect length for SetRequest", vbcounter, err_val );
            }
            else if( mibp->check )
            {
                i1 = mibp->check( mvp - mibp->mvp, tabix, inp );
                if( i1 )
                {
                    ERR_VGO( "MIB check invalidated SetRequest", vbcounter, err_val );
                }
            }
        }
        else
        {
            /* Perform read notification if possible and necessary */
            if( ( mvp->opt & CAR ) && mibp->get )
            {
                mibp->get( mvp - mibp->mvp, tabix, &valp );
            }
        }
        /* Table variables require an offset or explicit value */
        if( tabix >= 0 )
        {
            if( mvp->opt & SX )
#ifdef LITTLE
                valp = ( u8_t* )&tabix; /* Point to low order byte */
#else
                valp = ( u8_t* )&tabix + sizeof( tabix ) - 1;
#endif
            else if( ( ( mvp->opt & SCALAR ) == 0 ) && !( ( mvp->opt & CAR ) && mibp->get ) )
            {
                valp += mtp->len * tabix;
            }
        }
        /* Check the storage options */
        switch( mvp->opt & 7 )
        {
            case IMMED:
                ul1 = mvp->len;
                snmpRWriteInt( &outp, ul1, mvp->type, sizeof( mvp->type ) );
                break;
            case IMMED2:
                ul1 = ( ( u32_t )mvp->type << 8 ) + mvp->len;
                snmpRWriteInt( &outp, ul1, Integer, 2 );
                break;
            case BASE1:
                if( type == SetRequest )
                {
                    *valp = *chkp2 - 1;
                }
                snmpRWriteInt( &outp, ( *valp ) + 1, mvp->type, mvp->len );
                break;
            case 0:
            case SCALAR:
                cp = valp;
                switch( mvp->type )
                {
                    case Integer:
                    case Counter:
                    case Ticks:
                    case Gauge:
                        if( type == SetRequest )
                        {
                            len = snmpReadInt( &ul1, mvp->len, &inp, mvp->type );
                            if( len < 1 )
                            {
                                ERR_VGO( "Bad type/length field for var", vbcounter, err_parse );
                            }
                            i2 = ( mvp->len > len ) ? len : mvp->len;
                            i1 = mvp->len - i2;
#ifdef LITTLE
                            if( ( mvp->opt & NWORDER ) == 0 )
                            {
                                /* Fill low order bytes with value */
                                while( i2-- )
                                {
                                    *cp++ = ul1;
                                    ul1 >>= 8;
                                }
                                /* Fill high order bytes with sign-ness */
                                while( i1-- )
                                {
                                    *cp++ = ( u8_t )i3;
                                }
                            }
                            else
#endif
                            {
                                cp += mvp->len;
                                /* Fill low order bytes with value */
                                while( i2-- )
                                {
                                    *--cp = ul1;
                                    ul1 >>= 8;
                                }
                                /* Fill high order bytes with sign-ness */
                                while( i1-- )
                                {
                                    *--cp = ( u8_t )i3;
                                }
                            }
                            cp = valp;
                        }
                        if( mvp->opt & SX )
                        {
                            ul1 = tabix + 1;
                        }
                        else
                        {
                            ul1 = 0;
#ifdef LITTLE
                            if( ( mvp->opt & NWORDER ) == 0 )
                                for( i1 = mvp->len; i1--; )
                                {
                                    ul1 = ( ul1 << 8 ) + cp[i1];
                                }
                            else
#endif
                                for( i1 = mvp->len; i1; i1-- )
                                {
                                    ul1 = ( ul1 << 8 ) + *cp++;
                                }
                        }
                        snmpRWriteInt( &outp, ul1, mvp->type, mvp->len );
                        break;
                    case String:
                        /* Add one level of indirection for strings */
                        //cp = *(uint8 **)valp;
                        cp = ( u8_t* )valp;
                        if( type == SetRequest )
                        {
                            len = snmpReadVal( cp, mvp->len, &inp, mvp->type );
                            if( len < 0 || len > mvp->len )
                            {
                                ERR_VGO( "Bad type/length field for var", vbcounter, err_parse );
                            }
                            cp[len] = 0;
                        }
                        snmpRWriteVal( &outp, cp, String, slength( cp ) );
                        break;
                    default:
                        if( mvp->type == OctetString )
                        {
                            i2 = mvp->type;
                            if( type == SetRequest )
                            {
                                len = snmpReadVal( cp, mvp->len, &inp, String );
                                if( len < 0 || len > mvp->len )
                                {
                                    ERR_VGO( "Cannot set var", vbcounter, err_parse );
                                }
                            }
                            snmpRWriteVal( &outp, valp, String, mvp->len );
                            break;
                        }
                        i2 = mvp->type;
                        if( type == SetRequest )
                        {
                            len = snmpReadVal( cp, mvp->len, &inp, i2 );
                            if( len < 0 || len > mvp->len )
                            {
                                ERR_VGO( "Cannot set var", vbcounter, err_parse );
                            }
                        }
                        snmpRWriteVal( &outp, valp, i2, mvp->len );
                        break;
                }
                break;
            default:
                i1 = genErr;
                ERR_VGO( "Invalid options for var", vbcounter, err_val );
        }
        /* Perform write notification if possible and necessary */
        if( type == SetRequest && mvp->opt & CAW && mibp->set )
        {
            i1 = mibp->set( mvp - mibp->mvp, tabix );
            if( i1 < 0 )
            {
                outp = soutp;    /* value field */
            }
        }
        else
        {
            i1 = 0;
        }
        if( i1 < 0 )
        {
            ERR_VGO( "MIB set() failed", i1, err_val );
err_val:
            /* Version 2/3 doesn't set errcode for noSuchName */
            if( !version || i1 != noSuchName )
            {
                errcode = i1;   /* Saved from before goto! */
                errix = vbcounter;
            }
            snmpRWriteVal( &outp, 0, version ? ( nflag ? 0x82 : 0x81 ) : Null, 0 );
            snmpRWriteVal( &outp, inp, Identifier, len2 );  /* Input ID */
            mvp = 0;
        }
        else
        {
            if( tabix < 0 )
            {
                snmpRWriteVal( &outp, mvp->oid.name, Identifier, mvp->oid.nlen );
            }
            else
            {
                chkp = outp;
                snmpEncodeIndex( &outp, mibp, mtp, tabix, mvp );
                snmpRWriteVal( &outp, mvp->oid.name, Identifier, mvp->oid.nlen );
                /* The length must be reapplied to account for the table index */
                snmpReadLength( ( const u8_t** )&outp, Identifier ); //移动指针
                snmpRWriteLength( &outp, Identifier, chkp - outp );
            }
        }
        snmpRWriteLength( &outp, Sequence, soutp - outp );
        //vindex--;
        
        //BulkRep_Break(bulkflag, maxrep, nonrep, vbcounter, repcounter)? vbcounter++ : NULL;
        vbcounter++ ;
        
    }
    /* Use requested var-binds for bad v1 packet */
    if( version == 0 && errcode )
    {
        /* Point to start of first var-bind */
        i1 = ibpend - varbinds[0];
        outp = *obp - i1;
        memcpy( outp, varbinds[0], i1 );
    }
    /* Format reply ... */
    snmpRWriteLength( &outp, Sequence, *obp - outp ); /* Length of varbinds */
    if( errcode == 0 )
    {
        errix = 0;
    }
    else if( version == 1 )
    {
        switch( errcode )
        {
            case wrongValue:
            case wrongEncoding:
            case wrongType:
            case wrongLength:
            case inconsistentValue:
                errcode = badValue;
                break;
            case noAccess:
            case notWritable:
            case noCreation:
            case inconsistentName:
                errcode = noSuchName;
                break;
            case resourceUnavailable:
            case commitFailed:
            case undoFailed:
                errcode = genErr;
                break;
            case authorizationError:
                errcode = noSuchName;
                break;
            default:
                break;
        }
    }
    snmpRWriteInt( &outp, errix, Integer, sizeof( errix ) );     /* Error index */
    snmpRWriteInt( &outp, errcode, Integer, sizeof( errcode ) ); /* Error status */
    snmpRWriteInt( &outp, reqid, Integer, sizeof( reqid ) );     /* Request Id */
    snmpRWriteLength( &outp, GetResponse, *obp - outp );         /* PDU type */
    snmpRWriteVal( &outp, contextName, String, slength( contextName ) );
    snmpRWriteInt( &outp, version, Integer, sizeof( version ) ); /* Version */
    snmpRWriteLength( &outp, Sequence, *obp - outp );
    i1 = *obp - outp;
    *obp = outp;
    return i1;
}

