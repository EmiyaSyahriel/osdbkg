#r "nuget: SharpZipLib, 1.3.3"

using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Globalization;
using ICSharpCode.SharpZipLib.Zip.Compression.Streams;

bool DoNotUpdateIfSrcNewer = true;
string resdir = "src/res";
string targetHPP = "R.HPP";
string targetCPP = "R.CPP";
string[] fileNames = new[] { "MAIN.FRAG", "MAIN.VERT", "TEXIMAGE.TEX", "MODELS.OBJ", "LINE.FRAG", "LINE.VERT" };

string preamble =
@"//////////////////////////////
/// Auto-generated resources! please do-not edit!
//////////////////////////////
",
hpp_start = @"#pragma once
#ifndef R_HPP
#define R_HPP

extern const struct resource_t {
",
hpp_end = 
@" } R;
#endif
",
cpp_start = @"#include ""R.HPP""

const struct resource_t R = {
    ",
cpp_end = @"
};";

const string ERR_NOT_FOUND = "This file required for build process! please re-fetch or re-clone the repository.";

StringBuilder cpp = new StringBuilder(), hpp = new StringBuilder();

void PreproText(string file)
{
    cpp.Append("R\"GENRES(")
        .Append(File.ReadAllText(file))
        .Append(")GENRES\"");
}

// Text (0-F) -> Combine bytes -> RLE
void PreproTexImage(string file)
{
    var src = File.ReadAllText(file);
    List<byte> bytes = new List<byte>();
    { 
        byte a = 0, b = 0, c = 0;
        bool isSecond = false;

        for(int i =0;i < src.Length; i++)
        {
            char ch = src[i];
            ch = (char)(ch >= 'A' ? ((ch - 'A') + 0x0A) : (ch - '0'));
            if(ch <= 0x0F)
            {
                if (isSecond)
                {
                    b = (byte)ch;
                    c = (byte)((a << 4) | (b << 0));
                    bytes.Add(c);
                }
                else a = (byte)ch;
                isSecond = !isSecond;
            }
        }
    }

    byte[] deflateData;
    using (var outStream = new MemoryStream())
    {
        using (var deflater = new DeflaterOutputStream(outStream))
        {
            deflater.Write(bytes.ToArray(), 0, bytes.Count);
        }

        deflateData = outStream.ToArray();
    }
    cpp.Append("\"");
    for (int i =0; i < deflateData.Length; i++)
    {
        cpp.Append($"\\x{deflateData[i]:X2}");
        if ((i + 1) % 20 == 0) cpp.AppendLine("\"").Append("\t\"");
    }

    cpp.Append($"\",{deflateData.Length}");
}

static class Obj
{
    public class Vector2 { public float X, Y; 
        public Vector2(float x, float y) { X = x; Y = y; }
    }
    public class Vector3 { public float X, Y, Z;
        public Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
    }
    public class FaceIdx { public int Position, Texcoord, Normal; 
        public FaceIdx(int v, int vt, int vn) { Position = v; Texcoord = vt; Normal = vn; }
    }
    public class Face : List<FaceIdx> { }
    public class Object { public string Name; public List<Face> Faces = new List<Face>(); }
}

/**
 Model Data:
 int32 header - [OGLX]
 uint16 position_count, normal_count, texcoord_count, object_count;
 uint8 positions[3][vertice_count];
 uint8 normals[3][normal_count];
 uint8 texcoords[2][texcoord_count];
 struct object_t {
    uint8 name_len;
    uint8 face_len;
    char name[name_len];
    struct vertice_t {
        uint16 position_i, texcoord_i, normal_i;
     } faces[3][face_len];
 } objects[object_count];
 int32 footer - [EMSR]

 float data represented in uint8
 0   = -1.0
 128 = 0.0
 255 = 1.0
 */
void PreproObj(string file)
{
    string[] lines = File.ReadAllLines(file);
    List<Obj.Object> Objects = new List<Obj.Object>();
    List<Obj.Vector3> 
        Pos = new List<Obj.Vector3>(), 
        Norms = new List<Obj.Vector3>();
    List<Obj.Vector2> UVs = new List<Obj.Vector2>();

    Obj.Object activeObj = null;

    float SToF(string src, float dv = 0.0f)
    {
        float d = dv;
        if (!float.TryParse(src, System.Globalization.NumberStyles.Any, CultureInfo.InvariantCulture, out d))
        {
            d = dv;
        }
        
        return d;
    }

    void Push()
    {
        if(activeObj != null)
        {
            Objects.Add(activeObj);
        }
        activeObj = new Obj.Object();
    }

    #region OBJ Parse
    int lineNum = 0;
    foreach (string utline in lines)
    {
        lineNum++;
        string line = utline.Trim();
        if(line.Length > 0)
        {
            string[] fmt = line.Split(' ');
            switch (fmt[0])
            {
                case "v":
                    {
                        Pos.Add(new Obj.Vector3(SToF(fmt[1]), SToF(fmt[2]), SToF(fmt[3])));
                    }
                    break;
                case "vt":
                    {
                        UVs.Add(new Obj.Vector2(SToF(fmt[1]), SToF(fmt[2])));
                    }
                    break;
                case "vn":
                    {
                        Norms.Add(new Obj.Vector3(SToF(fmt[1]), SToF(fmt[2]), SToF(fmt[3])));
                    }
                    break;
                case "o":
                    {
                        Push();
                        activeObj.Name = fmt[1];
                    }
                    break;
                case "f":
                    {
                        if(activeObj != null)
                        {
                            Obj.Face face = new Obj.Face();
                            for(int i = 1; i < fmt.Length; i++)
                            {
                                var ffmt = fmt[i].Trim().Split('/');
                                if(ffmt.Length >= 3)
                                {
                                    int v = 0, vt = 0, vn = 0;
                                    int.TryParse(ffmt[0], out v);
                                    int.TryParse(ffmt[1], out vt);
                                    int.TryParse(ffmt[2], out vn);
                                    face.Add(new Obj.FaceIdx(v, vt, vn));
                                }
                                else
                                {
                                    Console.WriteLine($"WARNING: Unusual ObjFace format\n at {file}:{lineNum}");
                                }
                            }
                            activeObj.Faces.Add(face);
                        }
                        else
                        {
                            Console.WriteLine($"WARNING: ObjFace declared before any ObjObject\n at {file}:{lineNum}");
                        }
                    }
                    break;
            }
        }
    }
    Push(); // Push last
    #endregion OBJ Parse

    #region Serialize to Model Data
    List<byte> bytes = new List<byte>();
    {
        byte F2B(float f)
        {
            float sf = (f + 1.0f) * 0.5f;
            sf = Math.Min(1.0f, Math.Max(0.0f, sf));
            sf *= 255;
            return (byte)Math.Round(sf);
        }

        bytes.AddRange(Encoding.ASCII.GetBytes("OGLX"));
        bytes.AddRange(BitConverter.GetBytes((short)Pos.Count));
        bytes.AddRange(BitConverter.GetBytes((short)Norms.Count));
        bytes.AddRange(BitConverter.GetBytes((short)UVs.Count));
        bytes.AddRange(BitConverter.GetBytes((short)Objects.Count));
        foreach (List<Obj.Vector3> veclist in new[] { Pos, Norms })
            foreach (Obj.Vector3 vec in veclist) {
                bytes.Add(F2B(vec.X));
                bytes.Add(F2B(vec.Y));
                bytes.Add(F2B(vec.Z));
            }
        foreach (Obj.Vector2 vec in UVs)
        {
            bytes.Add(F2B(vec.X));
            bytes.Add(F2B(vec.Y));
        }
        foreach(Obj.Object obj in Objects)
        {
            bytes.Add((byte)obj.Name.Length);
            bytes.Add((byte)obj.Faces.Count);
            bytes.AddRange(Encoding.ASCII.GetBytes(obj.Name));
            foreach(Obj.Face face in obj.Faces)
            {
                for(int i = 0; i < 3; i++)
                {
                    Obj.FaceIdx idx = face[i];
                    bytes.AddRange(BitConverter.GetBytes((short)(idx.Position - 1)));
                    bytes.AddRange(BitConverter.GetBytes((short)(idx.Texcoord - 1)));
                    bytes.AddRange(BitConverter.GetBytes((short)(idx.Normal - 1)));
                }
            }
        }
        bytes.AddRange(Encoding.ASCII.GetBytes("EMSR"));
    }

    cpp.Append("\"");
    for(int i = 0; i < bytes.Count; i++)
    {
        cpp.Append($"\\x{bytes[i]:X2}");
        if ((i + 1) % 20 == 0) cpp.Append("\"").AppendLine().Append("\t\"");
    }
    cpp.Append($"\",{bytes.Count}");
    File.WriteAllBytes($"{file}.BIN", bytes.ToArray());
    #endregion
}

void Main()
{
    hpp.Append(preamble);
    cpp.Append(preamble);
    hpp.Append(hpp_start);
    cpp.Append(cpp_start);
    var cppPath = Path.Combine(resdir, targetCPP);
    var hppPath = Path.Combine(resdir, targetHPP);

    bool shouldUpdate = false;
    DateTime cppFileTime = DateTime.MinValue;
    DateTime hppFileTime = DateTime.MinValue;
    if (File.Exists(cppPath)) cppFileTime = File.GetLastWriteTimeUtc(cppPath);
    if (File.Exists(hppPath)) hppFileTime = File.GetLastWriteTimeUtc(hppPath);
    DateTime latestSrcTime = cppFileTime > hppFileTime ? cppFileTime : hppFileTime;

    for (int i = 0; i < fileNames.Length; i++)
    {
        string fileName = fileNames[i];
        var filePath = Path.Combine(resdir, fileName);
        if (!File.Exists(filePath))
            throw new FileNotFoundException(ERR_NOT_FOUND, filePath);
        
        var rsrcFileTime = File.GetLastWriteTimeUtc(filePath);
        if (latestSrcTime < rsrcFileTime) {
            Console.WriteLine($"Newer resource file detected, regenerating source file");
            Console.WriteLine($"RES={rsrcFileTime} > CPP={latestSrcTime}");
            shouldUpdate = true; 
        }
    }

    if (!shouldUpdate && DoNotUpdateIfSrcNewer)
    {
        Console.WriteLine("GENRES.CSX : Source file is newer than the resource file, nothing to do.");
        return;
    }

    for (int i = 0; i < fileNames.Length; i++) 
    {
        string fileName = fileNames[i];
        var filePath = Path.Combine(resdir, fileName);
        if (!File.Exists(filePath))
            throw new FileNotFoundException( ERR_NOT_FOUND, filePath);

        var ext = Path.GetExtension(filePath);
        var varName = fileName
            .Replace(".", "_")
            .ToLowerInvariant();
        hpp.Append("\tconst char* ").Append(varName).AppendLine(";");

        if (ext.ToUpperInvariant() == ".TEX") { 
            PreproTexImage(filePath);
            hpp.Append("\tint ").Append($"{varName}_sz").AppendLine(";");
            Console.WriteLine($"Processing {filePath} ({ext}) as Texture");
        }
        else if (ext.ToUpperInvariant() == ".OBJ")
        {
            PreproObj(filePath);
            hpp.Append("\tint ").Append($"{varName}_sz").AppendLine(";");
            Console.WriteLine($"Processing {filePath} ({ext}) as 3D Model");
        }
        else { 
            PreproText(filePath);
            Console.WriteLine($"Processing {filePath} ({ext}) as Text");
        }

        if (i < fileNames.Length - 1) cpp.Append(",").AppendLine().Append("\t");
    }
    hpp.Append(hpp_end);
    cpp.Append(cpp_end);

    File.WriteAllText(cppPath, cpp.ToString());
    File.WriteAllText(hppPath, hpp.ToString());
    File.SetLastWriteTimeUtc(cppPath, DateTime.UtcNow);
    File.SetLastWriteTimeUtc(hppPath, DateTime.UtcNow);
}

Main();