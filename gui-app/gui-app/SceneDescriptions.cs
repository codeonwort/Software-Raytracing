using System.Text.Json;
using System.Text.Json.Serialization;

namespace GuiApp
{
    [JsonConverter(typeof(SceneDescriptionsConverter))]
    internal class SceneDescriptions
    {
        public class Scene
        {
            [JsonInclude] public string name;
            [JsonInclude] public string filepath;
            [JsonInclude] public vec3 cameraLocation;
            [JsonInclude] public vec3 cameraLookAt;
            // If string, replace with defaultSunIlluminance.
            [JsonInclude] public vec3 sunIlluminance;
            // If string, replace with defaultSunDirection.
            [JsonInclude] public vec3 sunDirection;
        };

        [JsonInclude] public vec3 defaultSunIlluminance;
        [JsonInclude] public vec3 defaultSunDirection;
        [JsonInclude] public Scene[] scenes;

        private class SceneDescriptionsConverter : JsonConverter<SceneDescriptions>
        {
            public override SceneDescriptions? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
            {
                JsonElement root = JsonElement.ParseValue(ref reader);

                Func<JsonElement, string, vec3> GetVec3 = (node, prop) => {
                    var floats = node.GetProperty(prop).Deserialize<float[]>();
                    return new(floats[0], floats[1], floats[2]);
                };

                SceneDescriptions doc = new();
                doc.defaultSunIlluminance = GetVec3(root, "defaultSunIlluminance");
                doc.defaultSunDirection = GetVec3(root, "defaultSunDirection");

                var sceneElements = root.GetProperty("scenes");
                int sceneCount = sceneElements.GetArrayLength();
                doc.scenes = new Scene[sceneCount];
                for (int i = 0; i < sceneCount; ++i)
                {
                    var elem = sceneElements[i];
                    doc.scenes[i] = new Scene();
                    ref var scene = ref doc.scenes[i];
                    scene.name = elem.GetProperty("name").GetString();
                    scene.filepath = elem.GetProperty("filepath").GetString();
                    scene.cameraLocation = GetVec3(elem, "cameraLocation");
                    scene.cameraLookAt = GetVec3(elem, "cameraLookAt");
                    if (elem.GetProperty("sunIlluminance").ValueKind == JsonValueKind.String)
                    {
                        scene.sunIlluminance = doc.defaultSunIlluminance;
                    }
                    else
                    {
                        scene.sunIlluminance = GetVec3(elem, "sunIlluminance");
                    }
                    if (elem.GetProperty("sunDirection").ValueKind == JsonValueKind.String)
                    {
                        scene.sunDirection = doc.defaultSunDirection;
                    }
                    else
                    {
                        scene.sunDirection = GetVec3(elem, "sunDirection");
                    }
                }

                return doc;
            }

            public override void Write(Utf8JsonWriter writer, SceneDescriptions value, JsonSerializerOptions options)
            {
                throw new NotImplementedException();
            }
        }
    }
}
