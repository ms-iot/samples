// Copyright (c) 2015, Microsoft Corporation
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
// IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
//

using System;
using System.Collections.ObjectModel;
using System.Diagnostics;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Markup;

namespace MyLivingRoom.Views
{
    /// <summary>
    /// A value converter that maps one value to another, possibly of a different type.
    /// </summary>
    /// <remarks>
    /// <para>This is intended to be used from XAML, with the element content being used to populate the collection
    /// exposed by the <see cref="Mappings"/> property.</para>
    /// </remarks>
    /// <example>
    /// <para>The following C# code defines an enumeration type and a mapping for the type:</para>
    /// <code>
    /// <![CDATA[
    ///     public enum Severity
    ///     {
    ///         Info,
    ///         Warning,
    ///         Error,
    ///     }
    ///
    ///     public class SeverityMapping : Mapping<Severity, object>
    ///     {
    ///     }
    /// ]]>
    /// </code>
    /// <para>The following example XAML fragment shows the use of <see cref="MappingConverter"/> to map the example
    /// enumeration values to image paths:</para>
    /// <code>
    /// <![CDATA[
    ///     <UserControl.Resources>
    ///         <r:MappingConverter x:Key="severityToImageMappingConverter">
    ///             <r:SeverityMapping Key="Info" Value="images/InfoIcon.png" />
    ///             <r:SeverityMapping Key="Warning" Value="images/WarningIcon.png" />
    ///             <r:SeverityMapping Key="Error" Value="images/ErrorIcon.png" />
    ///         </r:MappingConverter>
    ///     </UserControl.Resources>
    /// ]]>
    /// </code>
    /// </example>
    [ContentProperty(Name = "Mappings")]
    public class MappingConverter : IValueConverter
    {
        #region Constructors

        /// <summary>
        /// Initializes a new instance of the <see cref="MappingConverter"/> class.
        /// </summary>
        public MappingConverter()
        {
            this.Mappings = new MappingCollection();
        }

        #endregion Constructors

        #region Properties

        #region DefaultValue

        public object DefaultValue
        {
            get { return _defaultValue; }
            set
            {
                _defaultValue = value;
                _hasDefaultValueBeenSet = true;
            }
        }
        private object _defaultValue;
        private bool _hasDefaultValueBeenSet;

        #endregion DefaultValue

        #region Mappings

        /// <summary>
        /// Gets the collection of mappings.
        /// </summary>
        /// <value>An instance of type <see cref="MappingCollection"/>.</value>
        /// <seealso cref="IMapping"/>
        /// <seealso cref="Mapping"/>
        /// <seealso cref="Mapping{TKey, TValue}"/>
        /// <seealso cref="MappingCollection"/>
        public MappingCollection Mappings { get; private set; }

        #endregion Mappings

        #region KeyComparisonType

        /// <summary>
        /// Gets or sets a value indicating how keys are compared.
        /// </summary>
        public MappingKeyComparisonType KeyComparisonType
        {
            get { return _keyComparisonType; }
            set
            {
                _keyComparisonType = value;

                switch (value)
                {
                    default:
                    case MappingKeyComparisonType.Exact:
                        _keyStringComparision = null;
                        break;

                    case MappingKeyComparisonType.CaseSensitiveString:
                        _keyStringComparision = StringComparison.Ordinal;
                        break;

                    case MappingKeyComparisonType.CaseInsensitiveString:
                        _keyStringComparision = StringComparison.OrdinalIgnoreCase;
                        break;
                }
            }
        }
        private MappingKeyComparisonType _keyComparisonType;
        private StringComparison? _keyStringComparision;

        #endregion KeyComparisonType

        #endregion Properties

        #region IValueConverter Members

        /// <summary>
        /// Modifies the source data before passing it to the target for display in the UI.
        /// </summary>
        /// <param name="value">The source data being passed to the target.</param>
        /// <param name="targetType">The <see cref="T:System.Type"/> of data expected by the target dependency property.</param>
        /// <param name="parameter">An optional parameter to be used in the converter logic.</param>
        /// <param name="culture">The culture of the conversion.</param>
        /// <returns>
        /// The value to be passed to the target dependency property.
        /// </returns>
        public object Convert(object value, Type targetType, object parameter, string language)
        {
            Debug.WriteLine("MappingConverter.Convert: value={0} targetType={1}", value, targetType.Name);

            for (var index = 0; index < this.Mappings.Count; ++index)
            {
                var mapping = this.Mappings[index];
                Debug.WriteLine("    key={0} value={1} ({2})", mapping.Key, mapping.Value, mapping.Value.GetType().Name);

                if (mapping.Key.Equals(value))
                {
                    return mapping.Value;
                }
            }

            if (_keyStringComparision != null && value != null)
            {
                var valueString = value.ToString();
                for (var index = 0; index < this.Mappings.Count; ++index)
                    if (valueString.Equals(this.Mappings[index].Key.ToString(), _keyStringComparision.Value))
                        return this.Mappings[index].Value;
            }

            return _hasDefaultValueBeenSet ? DefaultValue : value;
        }

        /// <summary>
        /// Modifies the target data before passing it to the source object.  This method is called only in
        /// <see cref="F:System.Windows.Data.BindingMode.TwoWay"/> bindings.
        /// </summary>
        /// <param name="value">The target data being passed to the source.</param>
        /// <param name="targetType">The <see cref="T:System.Type"/> of data expected by the source object.</param>
        /// <param name="parameter">An optional parameter to be used in the converter logic.</param>
        /// <param name="culture">The culture of the conversion.</param>
        /// <returns>
        /// The value to be passed to the source object.
        /// </returns>
        /// <exception cref="NotImplementedException">Always. Two-way conversion is not supported.</exception>
        public object ConvertBack(object value, Type targetType, object parameter, string language)
        {
            throw new NotImplementedException();
        }

        #endregion IValueConverter Members
    }

    public enum MappingKeyComparisonType
    {
        Exact,
        CaseSensitiveString,
        CaseInsensitiveString,
    }

    /// <summary>
    /// Defines a simple key/value object pair.
    /// </summary>
    /// <seealso cref="Mapping"/>
    /// <seealso cref="Mapping{TKey, TValue}"/>
    /// <seealso cref="MappingCollection"/>
    /// <seealso cref="MappingConverter"/>
    public interface IMapping
    {
        /// <summary>
        /// Gets the key in the key/value pair.
        /// </summary>
        object Key { get; }

        /// <summary>
        /// Gets the value in the key/value pair.
        /// </summary>
        object Value { get; }
    }

    /// <summary>
    /// Provides a generically typed implementation of the <see cref="IMapping"/> interface,
    /// which defines a simple key/value object pair.
    /// </summary>
    /// <typeparam name="TKey">The type of the key.</typeparam>
    /// <typeparam name="TValue">The type of the value.</typeparam>
    /// <seealso cref="IMapping"/>
    /// <seealso cref="Mapping"/>
    /// <seealso cref="MappingCollection"/>
    /// <seealso cref="MappingConverter"/>
    [ContentProperty(Name = "Value")]
    public class Mapping<TKey, TValue> : IMapping
    {
        #region Properties
        /// <summary>
        /// Gets or sets the key in the key/value pair.
        /// </summary>
        /// <value></value>
        public TKey Key { get; set; }

        /// <summary>
        /// Gets or sets the value in the key/value pair.
        /// </summary>
        /// <value></value>
        public TValue Value { get; set; }
        #endregion Properties

        #region IMapping interface
        object IMapping.Key { get { return Key; } }
        object IMapping.Value { get { return Value; } }
        #endregion IMapping interface
    }

    /// <summary>
    /// An ordered collection of <see cref="IMapping"/> objects.
    /// </summary>
    /// <seealso cref="IMapping"/>
    /// <seealso cref="Mapping"/>
    /// <seealso cref="Mapping{TKey, TValue}"/>
    /// <seealso cref="MappingConverter"/>
    public class MappingCollection : Collection<IMapping>
    {
    }

    /// <summary>
    /// Implements the <see cref="Mapping{TKey, TValue}"/> generic type for an untyped key/value pair.
    /// </summary>
    /// <seealso cref="IMapping"/>
    /// <seealso cref="Mapping{TKey, TValue}"/>
    /// <seealso cref="MappingCollection"/>
    /// <seealso cref="MappingConverter"/>
    public class Mapping : Mapping<object, object>
    {
    }

    /// <summary>
    /// Implements the <see cref="Mapping{TKey, TValue}"/> generic type for a Boolean key to an untyped value.
    /// </summary>
    /// <seealso cref="IMapping"/>
    /// <seealso cref="Mapping{TKey, TValue}"/>
    /// <seealso cref="MappingCollection"/>
    /// <seealso cref="MappingConverter"/>
    public class BooleanMapping : Mapping<bool, object>
    {
    }

    /// <summary>
    /// Implements the <see cref="Mapping{TKey, TValue}"/> generic type for an Int32 key to an untyped value.
    /// </summary>
    /// <seealso cref="IMapping"/>
    /// <seealso cref="Mapping{TKey, TValue}"/>
    /// <seealso cref="MappingCollection"/>
    /// <seealso cref="MappingConverter"/>
    public class Int32Mapping : Mapping<int, object>
    {
    }
}
